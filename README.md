# ClonoTribe - KORG monotribe VCV Rack Module

A module heavily inspired by the KORG monotribe analog synthesizer, including the firmware 2.1 features.

Please be aware that this is still work in progress. Specially in achieving the authentical sound there are a lot of improvements needed. Everyone is invited to bring this to perfection.

The monotribe is a great little box and I hope that this tribute will bring some fun to the community.

I'm currentyly working on improving the filter. Other parts also need improvements. There is also documentation missing.


## Features

- **Analog-style VCO**: Square, Triangle, and Sawtooth waveforms with octave control
- **MS-20 Filter**: Classic Korg low-pass filter with cutoff and resonance controls
- **LFO**: Multi-waveform LFO with  multiple targets and modes
- **Envelope Generator**: Attack/Gate/Decay modes
- **Simple Drum Machine**: Kick, Snare, and Hi-hat
- **8-step Sequencer**: Full recording, playback, and pattern editing (expandable to 16 steps)
- **Ribbon Controller**: Multi-mode pitch control
- **CV/Gate Integration**: Full CV input/output with recording capabilities

## Components

### VCO (Voltage Controlled Oscillator)
- Square, Triangle, and Sawtooth waveforms
- 5-octave range control
- CV input with 1V/octave standard
- Noise generator with level control

### VCF (Voltage Controlled Filter) 
- MS-20 style low-pass filter
- Cutoff frequency control (80Hz - 8kHz)
- Resonance (Peak) control up to self-oscillation
- LFO modulation support

### LFO (Low Frequency Oscillator)
- Multiple waveforms: Square, Triangle, Sawtooth, Sample & Hold
- Three modes: 1-Shot, Slow (0.05-18Hz), Fast (1-5000Hz)
- Three targets: VCF only, VCO+VCF, VCO only
- Rate and Intensity controls

### Envelope Generator
- Three modes matching original: Attack, Gate, Decay
- Automatic triggering from gate/ribbon input
- LFO trigger integration in 1-Shot mode

### Drum Machine
- **Kick Drum**: Analog-style bass drum with proper volume
- **Snare Drum**: Authentic snare with noise and tone components
- **Hi-Hat**: Metallic hi-hat sound with noise filtering
- Individual 8-step patterns per drum part
- Volume control and mixing

### Sequencer
- 8-step sequencer with individual step control (switchable to 16 steps)
- Record mode: live recording of CV/Gate and ribbon input
- Flux mode: Almost real-time step editing while playing
- Individual Active Step control per part
- Gate Time adjustment per step
- Tempo control (60-180 BPM up to 10 - 600 BPM) or external sync

### Ribbon Controller (Firmware 2.1 Features)
- Three modes: Key (chromatic), Narrow (±0.5 oct), Wide (±3 oct)
- **Gate Time Hold**: Real-time gate time modulation for synth
- **Volume Automation**: ±50% volume control for synth part
- **Drum Rolls**: Variable intensity drum rolls for drum parts
- CV output and gate generation

### CV/Gate Integration
- **CV Input**: 1V/octave pitch input with recording capability
- **Gate Input**: Trigger input for envelopes and recording
- **CV Output**: Outputs current pitch (ribbon, CV input, or sequencer)
- **Gate Output**: Outputs current gate state with proper timing
- **Audio Input**: External audio mixing capability
- **Sync Input/Output**: External sync or internal sync generation

## Usage

### Basic Operation
1. **Select Part**: Use SYNTH/KICK/SNARE/HIHAT buttons to select active part
2. **Pattern Programming**: Use step buttons 1-8 to program patterns
3. **Playback**: Press PLAY to start/stop sequencer
4. **Recording**: Press REC to record CV/Gate or ribbon input
5. **Flux Mode**: Press FLUX for real-time pattern editing

### Ribbon Controller
- **Synth Mode**: Touch ribbon for pitch control, hold GATE TIME for gate time modulation
- **Drum Mode**: Touch ribbon and hold GATE TIME for drum rolls
- **Volume Control**: In synth mode, ribbon position controls volume
- **Constant play**: Swipe down or up to keep the note playing

### Sequencer Features
- **Active Step**: Select step with step buttons, use ACTIVE STEP to toggle
- **Gate Time**: In synth mode, use GATE TIME to cycle through gate lengths
- **Recording**: Works with CV/Gate input, ribbon controller, or both
- **External Sync**: Connect clock to SYNC IN for external timing

## Building

This module requires VCV Rack SDK v2.

```bash
cd ClonoTribe
make clean
make
make install
```

## Dependencies

- VCV Rack SDK v2.x
- Standard C++ compiler (GCC/Clang/MSVC)
- Make build system

## Compatibility

- VCV Rack 2.x
- Linux, Windows, macOS
- All common sample rates (44.1kHz - 192kHz)

## License

GPL-3.0-or-later
