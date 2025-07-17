#!/usr/bin/env python3
"""
ClonoTribe Calibration Measurement Script

This script helps automate measurements for calibrating the ClonoTribe plugin
against a real Korg Monotribe.

Requirements:
- Python 3.6+
- numpy
- scipy  
- matplotlib
- pyaudio (for real-time analysis)

Usage:
python calibration_measurements.py --test vco --duration 5.0
"""

import argparse
import numpy as np
import matplotlib.pyplot as plt
from scipy import signal
from scipy.fft import fft, fftfreq
import json
import time
from datetime import datetime

class CalibrationMeasurement:
    def __init__(self, sample_rate=48000):
        self.sample_rate = sample_rate
        self.measurements = {}
        
    def analyze_frequency_response(self, audio_data, fundamental_freq=None):
        """Analyze frequency response of audio signal"""
        
        # Compute FFT
        fft_data = fft(audio_data)
        freqs = fftfreq(len(audio_data), 1/self.sample_rate)
        
        # Get positive frequencies only
        positive_freqs = freqs[:len(freqs)//2]
        magnitude = np.abs(fft_data[:len(fft_data)//2])
        
        # Convert to dB
        magnitude_db = 20 * np.log10(magnitude + 1e-12)
        
        # Find fundamental frequency if not provided
        if fundamental_freq is None:
            peak_idx = np.argmax(magnitude_db[1:]) + 1  # Skip DC
            fundamental_freq = positive_freqs[peak_idx]
            
        # Find harmonics
        harmonics = []
        for harmonic_num in range(1, 6):  # Up to 5th harmonic
            target_freq = fundamental_freq * harmonic_num
            idx = np.argmin(np.abs(positive_freqs - target_freq))
            harmonics.append({
                'harmonic': harmonic_num,
                'frequency': positive_freqs[idx],
                'magnitude_db': magnitude_db[idx]
            })
            
        return {
            'fundamental_freq': fundamental_freq,
            'harmonics': harmonics,
            'frequencies': positive_freqs,
            'magnitude_db': magnitude_db
        }
    
    def measure_envelope_timing(self, audio_data, gate_data):
        """Measure envelope attack and decay times"""
        
        # Find gate transitions
        gate_diff = np.diff(gate_data)
        gate_on_indices = np.where(gate_diff > 0.5)[0]
        gate_off_indices = np.where(gate_diff < -0.5)[0]
        
        if len(gate_on_indices) == 0 or len(gate_off_indices) == 0:
            return None
            
        measurements = []
        
        for gate_on in gate_on_indices:
            # Find corresponding gate off
            gate_off_candidates = gate_off_indices[gate_off_indices > gate_on]
            if len(gate_off_candidates) == 0:
                continue
            gate_off = gate_off_candidates[0]
            
            # Extract envelope segment
            envelope_segment = np.abs(audio_data[gate_on:gate_off])
            
            if len(envelope_segment) < 10:
                continue
                
            # Measure attack time (10% to 90% of peak)
            peak_value = np.max(envelope_segment)
            attack_10_percent = peak_value * 0.1
            attack_90_percent = peak_value * 0.9
            
            attack_start_idx = np.where(envelope_segment >= attack_10_percent)[0]
            attack_end_idx = np.where(envelope_segment >= attack_90_percent)[0]
            
            if len(attack_start_idx) > 0 and len(attack_end_idx) > 0:
                attack_time = (attack_end_idx[0] - attack_start_idx[0]) / self.sample_rate
            else:
                attack_time = None
                
            # Measure decay time after gate off
            if gate_off < len(audio_data) - 1000:  # Ensure we have enough samples
                decay_segment = np.abs(audio_data[gate_off:gate_off + 1000])
                
                if len(decay_segment) > 10:
                    decay_start = decay_segment[0]
                    decay_37_percent = decay_start * 0.37  # 1/e decay
                    
                    decay_idx = np.where(decay_segment <= decay_37_percent)[0]
                    if len(decay_idx) > 0:
                        decay_time = decay_idx[0] / self.sample_rate
                    else:
                        decay_time = None
                else:
                    decay_time = None
            else:
                decay_time = None
                
            measurements.append({
                'attack_time': attack_time,
                'decay_time': decay_time,
                'peak_value': peak_value
            })
            
        return measurements
    
    def measure_filter_response(self, input_audio, output_audio):
        """Measure filter frequency response using input/output comparison"""
        
        # Compute transfer function
        input_fft = fft(input_audio)
        output_fft = fft(output_audio)
        
        # Avoid division by zero
        transfer_function = np.divide(output_fft, input_fft, 
                                    out=np.zeros_like(output_fft), 
                                    where=np.abs(input_fft) > 1e-12)
        
        freqs = fftfreq(len(input_audio), 1/self.sample_rate)
        positive_freqs = freqs[:len(freqs)//2]
        magnitude_db = 20 * np.log10(np.abs(transfer_function[:len(transfer_function)//2]) + 1e-12)
        
        # Find cutoff frequency (-3dB point)
        max_magnitude = np.max(magnitude_db)
        cutoff_threshold = max_magnitude - 3.0
        
        cutoff_indices = np.where(magnitude_db <= cutoff_threshold)[0]
        if len(cutoff_indices) > 0:
            cutoff_freq = positive_freqs[cutoff_indices[0]]
        else:
            cutoff_freq = None
            
        return {
            'frequencies': positive_freqs,
            'magnitude_db': magnitude_db,
            'cutoff_freq': cutoff_freq
        }
    
    def save_measurement(self, test_name, data, filename=None):
        """Save measurement data to JSON file"""
        
        if filename is None:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"calibration_{test_name}_{timestamp}.json"
            
        measurement_data = {
            'timestamp': datetime.now().isoformat(),
            'test_name': test_name,
            'sample_rate': self.sample_rate,
            'data': data
        }
        
        with open(filename, 'w') as f:
            json.dump(measurement_data, f, indent=2)
            
        print(f"Measurement saved to {filename}")
        return filename
    
    def plot_frequency_response(self, analysis_data, title="Frequency Response"):
        """Plot frequency response analysis"""
        
        plt.figure(figsize=(12, 8))
        
        plt.subplot(2, 1, 1)
        plt.semilogx(analysis_data['frequencies'], analysis_data['magnitude_db'])
        plt.title(f"{title} - Magnitude Response")
        plt.xlabel("Frequency (Hz)")
        plt.ylabel("Magnitude (dB)")
        plt.grid(True)
        plt.xlim(20, 20000)
        
        plt.subplot(2, 1, 2)
        # Plot harmonic content
        if 'harmonics' in analysis_data:
            harmonics = analysis_data['harmonics']
            harmonic_nums = [h['harmonic'] for h in harmonics]
            harmonic_mags = [h['magnitude_db'] for h in harmonics]
            
            plt.bar(harmonic_nums, harmonic_mags)
            plt.title("Harmonic Content")
            plt.xlabel("Harmonic Number")
            plt.ylabel("Magnitude (dB)")
            plt.grid(True)
        
        plt.tight_layout()
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='ClonoTribe Calibration Measurements')
    parser.add_argument('--test', choices=['vco', 'vcf', 'envelope', 'lfo', 'drums'], 
                       required=True, help='Type of test to perform')
    parser.add_argument('--duration', type=float, default=5.0, 
                       help='Recording duration in seconds')
    parser.add_argument('--sample-rate', type=int, default=48000, 
                       help='Sample rate for analysis')
    parser.add_argument('--input-file', type=str, 
                       help='Use audio file instead of live recording')
    parser.add_argument('--output-file', type=str, 
                       help='Save analysis results to file')
    
    args = parser.parse_args()
    
    # Create measurement instance
    calibration = CalibrationMeasurement(sample_rate=args.sample_rate)
    
    print(f"Starting {args.test} calibration test...")
    print("Make sure both Monotribe and ClonoTribe are set to identical parameters!")
    
    if args.input_file:
        # Load from file (implement as needed)
        print(f"Loading audio from {args.input_file}")
        # TODO: Implement file loading
    else:
        # Live recording (implement as needed)
        print(f"Recording for {args.duration} seconds...")
        print("Press Enter when ready to start recording...")
        input()
        # TODO: Implement live recording with pyaudio
        
    print("Analysis complete!")

if __name__ == "__main__":
    main()
