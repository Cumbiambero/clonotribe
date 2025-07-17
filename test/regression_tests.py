import unittest
import numpy as np
from pathlib import Path
import json

class ClonoTribeRegressionTests(unittest.TestCase):
    
    def setUp(self):
        """Load reference test signals and expected outputs"""
        self.test_data_dir = Path(__file__).parent / "reference_data"
        self.load_reference_data()
        
    def load_reference_data(self):
        """Load golden reference audio files"""
        self.reference_signals = {}
        for param_file in self.test_data_dir.glob("params_*.json"):
            with open(param_file) as f:
                params = json.load(f)
            audio_file = param_file.with_suffix('.wav')
            if audio_file.exists():
                # Load audio data
                self.reference_signals[param_file.stem] = {
                    'params': params,
                    'audio': self.load_audio(audio_file)
                }
    
    def test_filter_response_stability(self):
        """Ensure filter response hasn't changed"""
        for test_name, data in self.reference_signals.items():
            with self.subTest(test=test_name):
                current_output = self.generate_clonotribe_output(data['params'])
                similarity = self.calculate_similarity(data['audio'], current_output)
                self.assertGreater(similarity, 0.99, 
                    f"Filter response changed for {test_name}: similarity={similarity}")
    
    def test_parameter_ranges(self):
        """Test all parameters stay within valid ranges"""
        test_params = [
            {'param_0': 0.0, 'param_1': 0.0, 'param_2': 0.0, 'param_3': 0.0},
            {'param_0': 1.0, 'param_1': 1.0, 'param_2': 1.0, 'param_3': 1.0},
            {'param_0': 0.5, 'param_1': 0.5, 'param_2': 0.5, 'param_3': 0.5}
        ]
        
        for params in test_params:
            with self.subTest(params=params):
                output = self.generate_clonotribe_output(params)
                self.assertIsNotNone(output, "Module crashed with valid parameters")
                self.assertFalse(np.any(np.isnan(output)), "NaN values in output")
                self.assertFalse(np.any(np.isinf(output)), "Infinite values in output")
    
    def test_monotribe_reference_match(self):
        """Core test: ensure ClonoTribe matches MonoTribe reference"""
        monotribe_ref = self.load_audio(self.test_data_dir / "monotribe_reference.wav")
        optimal_params = self.load_json(self.test_data_dir / "optimal_params.json")
        
        clonotribe_output = self.generate_clonotribe_output(optimal_params)
        similarity = self.calculate_similarity(monotribe_ref, clonotribe_output)
        
        self.assertGreater(similarity, 0.995, 
            f"ClonoTribe doesn't match MonoTribe: similarity={similarity}")

if __name__ == '__main__':
    unittest.main()
