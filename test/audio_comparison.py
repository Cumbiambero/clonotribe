#!/usr/bin/env python3
"""
Audio Comparison Tool for ClonoTribe Calibration

This tool helps compare audio recordings from the real Monotribe
and ClonoTribe plugin to identify differences and optimize parameters.
"""

import numpy as np
import matplotlib.pyplot as plt
from scipy.io import wavfile
from scipy.fft import fft, fftfreq
from scipy.optimize import minimize
import argparse
import json
import logging
from pathlib import Path

class AudioComparator:
    def __init__(self, sample_rate=44100):
        self.sample_rate = sample_rate
        self.tolerance_db = 0.1  # Very tight tolerance for identical behavior
        
    def load_audio(self, filename):
        """Load audio file and return sample rate and audio data"""
        try:
            sample_rate, audio_data = wavfile.read(filename)
            # Convert to mono if stereo
            if len(audio_data.shape) > 1:
                audio_data = np.mean(audio_data, axis=1)
            # Normalize to float
            if audio_data.dtype == np.int16:
                audio_data = audio_data.astype(np.float32) / 32767.0
            elif audio_data.dtype == np.int32:
                audio_data = audio_data.astype(np.float32) / 2147483647.0
            return sample_rate, audio_data
        except Exception as e:
            print(f"Error loading {filename}: {e}")
            return None, None
    
    def load_reference_audio(self, filepath):
        """Load MonoTribe reference audio"""
        _, audio_data = self.load_audio(filepath)
        return audio_data
        
    def capture_clonotribe_audio(self, parameters):
        """Capture ClonoTribe audio with given parameters"""
        # TODO: Interface with VCV Rack via OSC or file export
        # For now, return placeholder
        logging.warning("capture_clonotribe_audio not implemented - using placeholder")
        return np.random.randn(44100)  # 1 second of noise as placeholder
        
    def analyze_frequency_spectrum(self, audio_data, sample_rate, window_size=4096):
        """Analyze frequency spectrum of audio signal"""
        window = np.hanning(window_size)
        num_chunks = len(audio_data) // window_size
        if num_chunks == 0:
            num_chunks = 1
            window_size = len(audio_data)
            window = np.hanning(window_size)
        
        spectra = []
        for i in range(num_chunks):
            start_idx = i * window_size
            end_idx = start_idx + window_size
            if end_idx > len(audio_data):
                break
                
            chunk = audio_data[start_idx:end_idx] * window
            spectrum = fft(chunk)
            spectra.append(np.abs(spectrum))
        
        avg_spectrum = np.mean(spectra, axis=0)
        freqs = fftfreq(window_size, 1/sample_rate)
        positive_freqs = freqs[:window_size//2]
        magnitude = avg_spectrum[:window_size//2]
        magnitude_db = 20 * np.log10(magnitude + 1e-12)
        
        return positive_freqs, magnitude_db
        
    def calculate_similarity_metrics(self, reference, test_signal):
        """Calculate comprehensive similarity metrics"""
        # Ensure signals are same length
        min_len = min(len(reference), len(test_signal))
        ref_trimmed = reference[:min_len]
        test_trimmed = test_signal[:min_len]
        
        metrics = {}
        
        # Frequency domain comparison
        ref_fft = np.fft.fft(ref_trimmed)
        test_fft = np.fft.fft(test_trimmed)
        metrics['spectral_correlation'] = np.corrcoef(np.abs(ref_fft), np.abs(test_fft))[0,1]
        
        # Time domain comparison
        metrics['time_correlation'] = np.corrcoef(ref_trimmed, test_trimmed)[0,1]
        
        # RMS difference
        metrics['rms_error'] = np.sqrt(np.mean((ref_trimmed - test_trimmed)**2))
        
        return metrics
        
    def optimize_parameters(self, reference_audio, initial_params):
        """Use optimization to find best matching parameters"""
        def objective(params):
            test_audio = self.capture_clonotribe_audio(params)
            metrics = self.calculate_similarity_metrics(reference_audio, test_audio)
            return 1.0 - metrics['spectral_correlation']  # Minimize dissimilarity
            
        result = minimize(objective, initial_params, method='Nelder-Mead')
        return result.x
    
    def compare_files(self, monotribe_file, clonotribe_file, analysis_type='spectrum', title='Audio Comparison'):
        """Compare two audio files (main interface function)"""
        print(f"Loading {monotribe_file}...")
        mono_sr, mono_audio = self.load_audio(monotribe_file)
        if mono_audio is None:
            return None
            
        print(f"Loading {clonotribe_file}...")
        clono_sr, clono_audio = self.load_audio(clonotribe_file)
        if clono_audio is None:
            return None
        
        if mono_sr != clono_sr:
            print(f"Warning: Sample rates don't match ({mono_sr} vs {clono_sr})")
        
        results = {}
        
        if analysis_type in ['spectrum', 'both']:
            print("Analyzing frequency spectra...")
            
            mono_freqs, mono_mag = self.analyze_frequency_spectrum(mono_audio, mono_sr)
            clono_freqs, clono_mag = self.analyze_frequency_spectrum(clono_audio, clono_sr)
            
            # Calculate similarity metrics
            metrics = self.calculate_similarity_metrics(mono_audio, clono_audio)
            results['metrics'] = metrics
            
            # Plot comparison
            self._plot_spectrum_comparison(mono_freqs, mono_mag, clono_freqs, clono_mag, title)
            
            print(f"Spectral correlation: {metrics['spectral_correlation']:.4f}")
            print(f"Time correlation: {metrics['time_correlation']:.4f}")
            print(f"RMS error: {metrics['rms_error']:.6f}")
        
        return results
    
    def _plot_spectrum_comparison(self, freqs1, mag1, freqs2, mag2, title):
        """Plot comparison of two frequency spectra"""
        plt.figure(figsize=(12, 8))
        
        plt.subplot(2, 1, 1)
        plt.semilogx(freqs1, mag1, label='Monotribe', alpha=0.8, color='blue')
        plt.semilogx(freqs2, mag2, label='ClonoTribe', alpha=0.8, color='red')
        plt.title(f"{title} - Frequency Spectra")
        plt.xlabel("Frequency (Hz)")
        plt.ylabel("Magnitude (dB)")
        plt.legend()
        plt.grid(True, alpha=0.3)
        plt.xlim(20, 20000)
        
        plt.subplot(2, 1, 2)
        common_freqs = freqs1
        mag2_interp = np.interp(common_freqs, freqs2, mag2)
        difference = mag1 - mag2_interp
        
        plt.semilogx(common_freqs, difference, color='green', alpha=0.8)
        plt.title("Difference (Monotribe - ClonoTribe)")
        plt.xlabel("Frequency (Hz)")
        plt.ylabel("Magnitude Difference (dB)")
        plt.grid(True, alpha=0.3)
        plt.xlim(20, 20000)
        plt.axhline(y=0, color='black', linestyle='--', alpha=0.5)
        
        plt.tight_layout()
        plt.show()

def main():
    parser = argparse.ArgumentParser(description='Compare Monotribe and ClonoTribe audio recordings')
    parser.add_argument('monotribe_file', help='Audio file from Monotribe')
    parser.add_argument('clonotribe_file', help='Audio file from ClonoTribe')
    parser.add_argument('--analysis', choices=['spectrum', 'timing', 'both'], 
                       default='spectrum', help='Type of analysis to perform')
    parser.add_argument('--title', default='Audio Comparison', help='Title for plots')
    
    args = parser.parse_args()
    
    comparator = AudioComparator()
    results = comparator.compare_files(
        args.monotribe_file, 
        args.clonotribe_file, 
        args.analysis, 
        args.title
    )

if __name__ == "__main__":
    main()