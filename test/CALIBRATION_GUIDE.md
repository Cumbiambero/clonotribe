# ClonoTribe Calibration Guide

## Overview
This guide will help you calibrate the ClonoTribe plugin against a real Korg Monotribe to achieve accurate emulation.

## Required Equipment
- Real Korg Monotribe
- Audio interface with at least 2 inputs
- Oscilloscope or spectrum analyzer (optional but recommended)
- DAW with spectrum analysis capabilities
- Reference tones (sine wave generator)

## Measurement Setup

### Audio Chain
1. **Monotribe Output** → Audio Interface Input 1
2. **VCV Rack Output** → Audio Interface Input 2
3. Set both to same levels (use VU meters or oscilloscope)

### Basic Setup
1. Initialize both Monotribe and ClonoTribe to factory defaults
2. Set identical parameter values on both devices
3. Use identical input signals (CV/Gate from same source)

## Calibration Tests

### 1. VCO Calibration
**Purpose**: Match oscillator frequency response and waveforms

**Test Procedure**:
- Set VCO to middle octave (OCT = 2)
- Test each waveform (Square, Triangle, Sawtooth)
- Measure frequency accuracy across CV range
- Compare harmonic content using spectrum analysis

**Parameters to measure**:
- Frequency scaling (1V/Oct accuracy)
- Waveform shape and harmonic content
- Frequency stability
- Temperature drift (if applicable)

### 2. VCF Calibration
**Purpose**: Match filter frequency response and resonance

**Test Procedure**:
- Use white noise or sawtooth input
- Sweep CUTOFF from 0 to 100%
- Test PEAK (resonance) at different levels
- Measure frequency response curves

**Parameters to measure**:
- Cutoff frequency range
- Resonance behavior and self-oscillation point
- Filter slope (should be 24dB/octave)
- Frequency response curves

### 3. Envelope Calibration
**Purpose**: Match envelope timing and shapes

**Test Procedure**:
- Test each envelope form (Attack, Gate, Decay)
- Measure attack and decay times
- Test with different gate lengths

**Parameters to measure**:
- Attack times (typically 2ms to 4s)
- Decay times (typically 2ms to 4s)
- Envelope curves (exponential vs linear)

### 4. LFO Calibration
**Purpose**: Match LFO rates and waveforms

**Test Procedure**:
- Test each waveform and speed setting
- Measure actual frequencies vs control positions
- Test one-shot behavior

**Parameters to measure**:
- LFO frequency ranges
- Waveform accuracy
- One-shot timing

### 5. Sequencer Calibration
**Purpose**: Match sequencer timing and behavior

**Test Procedure**:
- Test tempo accuracy across range
- Verify 16-step mode behavior
- Test sync input/output

**Parameters to measure**:
- Tempo range and accuracy
- Gate timing precision
- Sync signal levels and timing

### 6. Drum Calibration
**Purpose**: Match drum sounds and timing

**Test Procedure**:
- Compare each drum sound in isolation
- Test drum rolls (flam length)
- Measure frequency content

**Parameters to measure**:
- Kick drum frequency and decay
- Snare noise characteristics and pitch
- Hi-hat frequency content and decay

## Measurement Tools

### Spectrum Analysis
Use these settings for consistent measurements:
- FFT Size: 8192 or higher
- Window: Hanning
- Sample Rate: 48kHz or higher
- Frequency range: 20Hz - 20kHz

### Timing Measurements
- Use gate/trigger outputs to sync measurements
- Measure rise/fall times with oscilloscope
- Check for timing jitter

## Expected Ranges (Reference Values)

### VCO
- Frequency range: ~32Hz to ~4kHz (approximate)
- 1V/Oct scaling: ±5 cents accuracy
- Waveform THD: <5% for fundamental

### VCF
- Cutoff range: ~100Hz to ~8kHz (approximate)
- Resonance: Up to self-oscillation
- Filter slope: 24dB/octave lowpass

### Envelope
- Attack/Decay range: ~2ms to ~4s
- Curve: Exponential

### LFO
- Rate range: ~0.1Hz to ~20Hz
- Waveforms: Square, Triangle, Sawtooth

### Drums
- Kick: Fundamental ~60-80Hz
- Snare: Noise + tone ~200Hz
- Hi-hat: High frequency noise ~8-12kHz

## Calibration Process

1. **Start with VCO**: Get basic pitch and waveforms right
2. **Tune VCF**: Match filter response curves
3. **Adjust Envelope**: Match timing characteristics
4. **Calibrate LFO**: Match modulation rates and shapes
5. **Fine-tune Drums**: Match spectral content
6. **Test Interactions**: Verify modulation behaviors

## Documentation
Record all measurements in spreadsheets with:
- Parameter settings
- Measured values
- Target values (from Monotribe)
- Adjustment needed
- Before/after comparisons

## Next Steps
After initial measurements, I'll help you:
1. Analyze the differences
2. Adjust the DSP algorithms
3. Update parameter ranges
4. Fine-tune response curves
5. Iterate until perfect match

Let's start with the VCO calibration - set both devices to identical settings and let me know what you measure!
