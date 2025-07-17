import numpy as np
from pathlib import Path
import logging
from audio_comparison import AudioComparator
import json

class CalibrationSuite:
    def __init__(self):
        self.comparator = AudioComparator()
        self.calibration_data = {}
        self.results_dir = Path(__file__).parent / "calibration_results"
        self.results_dir.mkdir(exist_ok=True)
        
    def run_full_calibration(self, monotribe_recordings_dir):
        """Run complete calibration against MonoTribe recordings"""
        logging.info("Starting full calibration suite...")
        
        recordings = list(Path(monotribe_recordings_dir).glob("*.wav"))
        
        for recording in recordings:
            logging.info(f"Calibrating against {recording.name}")
            
            # Extract test conditions from filename
            test_conditions = self.parse_filename(recording.name)
            
            # Load reference audio
            reference = self.comparator.load_reference_audio(recording)
            
            # Find optimal parameters
            initial_params = [2.0, 0.8, 0.0, 0.0]  # Starting point
            optimal_params = self.comparator.optimize_parameters(reference, initial_params)
            
            # Validate results
            validation_metrics = self.validate_calibration(reference, optimal_params)
            
            # Store results
            self.calibration_data[recording.stem] = {
                'test_conditions': test_conditions,
                'optimal_parameters': optimal_params.tolist(),
                'validation_metrics': validation_metrics,
                'target_similarity': validation_metrics['spectral_correlation']
            }
            
        self.save_calibration_results()
        return self.calibration_data
    
    def validate_calibration(self, reference, parameters):
        """Validate calibration quality"""
        test_output = self.comparator.capture_clonotribe_audio(parameters)
        metrics = self.comparator.calculate_similarity_metrics(reference, test_output)
        
        # Check if we achieved identical behavior
        if metrics['spectral_correlation'] > 0.995:
            logging.info("✓ Identical behavior achieved!")
        elif metrics['spectral_correlation'] > 0.99:
            logging.warning("△ Very close behavior achieved")
        else:
            logging.error("✗ Significant difference remains")
            
        return metrics
    
    def generate_regression_test_data(self):
        """Generate reference data for regression tests"""
        reference_dir = Path(__file__).parent / "reference_data"
        reference_dir.mkdir(exist_ok=True)
        
        for test_name, data in self.calibration_data.items():
            # Save optimal parameters
            param_file = reference_dir / f"params_{test_name}.json"
            with open(param_file, 'w') as f:
                json.dump(data['optimal_parameters'], f, indent=2)
                
            # Generate and save reference audio
            audio_output = self.comparator.capture_clonotribe_audio(data['optimal_parameters'])
            audio_file = reference_dir / f"audio_{test_name}.wav"
            self.save_audio(audio_output, audio_file)
            
        logging.info(f"Generated regression test data in {reference_dir}")
    
    def parse_filename(self, filename):
        """Extract test conditions from filename"""
        # Example: monotribe_cutoff_50_resonance_30.wav
        parts = filename.replace('.wav', '').split('_')
        conditions = {}
        for i in range(1, len(parts), 2):
            if i + 1 < len(parts):
                conditions[parts[i]] = float(parts[i + 1])
        return conditions
    
    def save_calibration_results(self):
        """Save calibration results to JSON file"""
        results_file = self.results_dir / "calibration_results.json"
        with open(results_file, 'w') as f:
            json.dump(self.calibration_data, f, indent=2)
        logging.info(f"Calibration results saved to {results_file}")
    
    def save_audio(self, audio_data, filepath):
        """Save audio data to WAV file"""
        import soundfile as sf
        sf.write(filepath, audio_data, self.comparator.sample_rate)

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
    
    # Check if monotribe recordings directory exists
    recordings_dir = Path("monotribe_recordings")
    if not recordings_dir.exists():
        recordings_dir.mkdir()
        print(f"Created directory: {recordings_dir}")
        print("Please place your MonoTribe recordings in this directory and run again.")
        exit(1)
    
    suite = CalibrationSuite()
    
    # Run calibration if recordings exist
    recordings = list(recordings_dir.glob("*.wav"))
    if recordings:
        print(f"Found {len(recordings)} MonoTribe recordings")
        results = suite.run_full_calibration(recordings_dir)
        suite.generate_regression_test_data()
        print("Calibration complete! Check calibration_results/ directory for results.")
    else:
        print("No .wav files found in monotribe_recordings/ directory")
        print("Please add MonoTribe recordings and run again.")
