# Test Sequences for ClonoTribe Calibration

This file contains test sequences and parameter settings for systematic calibration.

## VCO Calibration Tests

### Test 1: Frequency Accuracy
**Purpose**: Verify 1V/Oct scaling accuracy

**Setup**:
- ClonoTribe: VCO OCT = 2, Waveform = Square
- Monotribe: VCO OCT = 2, Waveform = Square
- Both: VCF CUTOFF = 100%, PEAK = 0%, VCA LEVEL = 80%

**Test Sequence**:
1. CV = 0V (should produce ~261Hz, C4)
2. CV = 1V (should produce ~523Hz, C5) 
3. CV = 2V (should produce ~1046Hz, C6)
4. CV = -1V (should produce ~131Hz, C3)

**Expected Results**:
- Frequency should double for each 1V increase
- Accuracy should be within ±5 cents

### Test 2: Waveform Comparison
**Purpose**: Match waveform shapes and harmonic content

**Setup**:
- CV = 1V (C5, ~523Hz)
- VCF CUTOFF = 100%, PEAK = 0%
- Test each waveform: Square, Triangle, Sawtooth

**Measurements**:
- Record 2-3 seconds of each waveform
- Analyze harmonic content with FFT
- Compare THD (Total Harmonic Distortion)

## VCF Calibration Tests

### Test 3: Filter Sweep
**Purpose**: Match filter frequency response

**Setup**:
- White noise input OR VCO sawtooth at 1kHz
- PEAK = 0% (no resonance)
- Sweep CUTOFF from 0% to 100%

**Test Points** (approximate):
- CUTOFF = 0%   → ~100Hz cutoff
- CUTOFF = 25%  → ~300Hz cutoff  
- CUTOFF = 50%  → ~800Hz cutoff
- CUTOFF = 75%  → ~2kHz cutoff
- CUTOFF = 100% → ~6kHz cutoff

### Test 4: Resonance Test
**Purpose**: Match resonance behavior

**Setup**:
- White noise input
- CUTOFF = 50% (mid position)
- Sweep PEAK from 0% to 100%

**Expected Behavior**:
- 0%: No resonance peak
- 50%: Moderate resonance peak
- 90%+: Filter should self-oscillate

## Envelope Calibration Tests

### Test 5: Envelope Timing
**Purpose**: Match envelope attack/decay times

**Setup**:
- VCO sawtooth, CUTOFF = 100%
- Gate length = 500ms
- Test each envelope form

**Test Sequence**:
1. ENV FORM = Attack: Measure attack time
2. ENV FORM = Gate: Should follow gate exactly
3. ENV FORM = Decay: Measure decay time

**Timing Measurements**:
- Attack: 10% to 90% rise time
- Decay: 90% to 10% fall time (after gate off)

## LFO Calibration Tests

### Test 6: LFO Rate Accuracy
**Purpose**: Match LFO frequencies

**Setup**:
- LFO → VCF (moderate intensity)
- VCO constant tone for reference
- LFO MODE = Fast

**Test Points**:
- RATE = 0%   → ~0.1Hz
- RATE = 25%  → ~1Hz
- RATE = 50%  → ~5Hz  
- RATE = 75%  → ~10Hz
- RATE = 100% → ~20Hz

### Test 7: LFO Waveforms
**Purpose**: Match LFO waveform shapes

**Setup**:
- LFO RATE = 1Hz (for easy measurement)
- Route LFO to VCF with high intensity
- Test each waveform

## Sequencer Calibration Tests

### Test 8: Tempo Accuracy
**Purpose**: Verify sequencer timing

**Setup**:
- Simple 4-step sequence
- All steps active
- Test different tempo settings

**Measurements**:
- TEMPO = 0%   → 60 BPM
- TEMPO = 50%  → 120 BPM  
- TEMPO = 100% → 300 BPM

**Timing Check**:
- Measure actual step intervals
- Compare with expected 16th note timing

### Test 9: 16-Step Mode
**Purpose**: Verify 16-step functionality

**Setup**:
- Enable 16-step mode (GATE_TIME + SEQ 6)
- Program different pattern for main/sub steps
- Verify LED behavior

## Drum Calibration Tests

### Test 10: Individual Drum Sounds
**Purpose**: Match drum spectral content

**Test Each Drum**:
1. **Kick Drum**: Measure fundamental frequency and decay
2. **Snare**: Analyze noise spectrum and pitch component  
3. **Hi-Hat**: Check high-frequency content and decay

**Setup**:
- RHYTHM VOLUME = 80%
- Single drum hits
- No effects or filtering

## Reference Measurements Template

Use this template to record your measurements:

```
Date: ___________
Test: ___________
Device: [Monotribe/ClonoTribe]

Parameter Settings:
- VCO OCT: ____
- VCO WAVE: ____
- VCF CUTOFF: ____
- VCF PEAK: ____
- VCA LEVEL: ____
- [other relevant params]

Measurements:
- Frequency: ______ Hz
- Amplitude: ______ dB
- THD: ______%
- Attack Time: ______ ms
- Decay Time: ______ ms
- [other measurements]

Notes:
________________
```

## Quick Start Procedure

1. **Set both devices to identical parameters**
2. **Record identical test signals** (use same CV/Gate source)
3. **Analyze recordings** with spectrum analyzer
4. **Document differences** in measurement template
5. **Adjust ClonoTribe parameters** to match Monotribe
6. **Repeat until satisfied**

Start with VCO frequency accuracy - this is the foundation for everything else!
