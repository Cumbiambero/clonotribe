#include "clonotribe.hpp"

Clonotribe::Clonotribe() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
               
        configSwitch(PARAM_VCO_WAVEFORM_SWITCH, 0.f, 2.f, 0.f, "VCO Waveform", {"Square", "Triangle", "Sawtooth"});
        configSwitch(PARAM_RIBBON_RANGE_SWITCH, 0.f, 2.f, 0.f, "Ribbon Range", {"Key", "Narrow", "Wide"});
        configSwitch(PARAM_ENVELOPE_FORM_SWITCH, 0.f, 2.f, 0.f, "Envelope",  {"Attack", "Gate", "Decay"});
        configSwitch(PARAM_LFO_TARGET_SWITCH, 0.f, 2.f, 0.f, "LFO Target",  {"VCF", "VCO+VCF", "VCO"});
        configSwitch(PARAM_LFO_MODE_SWITCH, 0.f, 2.f, 0.f, "LFO Mode", {"1 Shot", "Slow", "Fast"});
        configSwitch(PARAM_LFO_WAVEFORM_SWITCH, 0.f, 2.f, 0.f, "LFO Waveform",  {"Square", "Triangle", "Sawtooth"});
        
        configParam(PARAM_VCO_OCTAVE_KNOB, 0.f, 5.f, 2.f, "VCO Octave");
        getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->snapEnabled = true;

        configParam(PARAM_NOISE_KNOB, 0.f, 1.f, 0.f, "Noise Level", "%", 0.f, 100.f);
        configParam(PARAM_VCF_CUTOFF_KNOB, 0.f, 1.f, 0.7f, "VCF Cutoff");
        configParam(PARAM_VCF_PEAK_KNOB, 0.f, 1.f, 0.f, "VCF Peak (Resonance)");
        configParam(PARAM_VCA_LEVEL_KNOB, 0.f, 1.f, 0.8f, "VCA Level");
        configParam(PARAM_LFO_RATE_KNOB, 0.f, 1.f, 0.3f, "LFO Rate");
        configParam(PARAM_LFO_INTERVAL_KNOB, 0.f, 1.f, 0.f, "LFO Intensity");
        configParam(PARAM_RHYTHM_VOLUME_KNOB, 0.f, 1.f, 0.f, "Rhythm Volume");
        configParam(PARAM_SEQUENCER_TEMPO_KNOB, 0.f, 1.f, 0.5f, "Sequencer Tempo", " BPM", 0.f, 120.f, 60.f);
        
        configButton(PARAM_SNARE_BUTTON, "Snare");
        configButton(PARAM_FLUX_BUTTON, "Flux");
        configButton(PARAM_REC_BUTTON, "Record");
        configButton(PARAM_SYNTH_BUTTON, "Synth");
        configButton(PARAM_BASSDRUM_BUTTON, "Bass Drum");
        configButton(PARAM_HIGHHAT_BUTTON, "Hi-Hat");
        configButton(PARAM_ACTIVE_STEP_BUTTON, "Active Step (F7)");
        configButton(PARAM_SEQUENCER_1_BUTTON, "Sequencer 1");
        configButton(PARAM_SEQUENCER_2_BUTTON, "Sequencer 2");
        configButton(PARAM_SEQUENCER_3_BUTTON, "Sequencer 3");
        configButton(PARAM_SEQUENCER_4_BUTTON, "Sequencer 4");
        configButton(PARAM_SEQUENCER_5_BUTTON, "Sequencer 5");
        configButton(PARAM_SEQUENCER_6_BUTTON, "Sequencer 6");
        configButton(PARAM_SEQUENCER_7_BUTTON, "Sequencer 7");
        configButton(PARAM_SEQUENCER_8_BUTTON, "Sequencer 8");
        configButton(PARAM_PLAY_BUTTON, "Play");
        configButton(PARAM_GATE_TIME_BUTTON, "Gate Time (F8)");
        
        configInput(INPUT_CV_CONNECTOR, "CV");
        configInput(INPUT_GATE_CONNECTOR, "Gate");
        configInput(INPUT_AUDIO_CONNECTOR, "Audio");
        configInput(INPUT_SYNC_CONNECTOR, "Sync");
        
        configOutput(OUTPUT_CV_CONNECTOR, "CV");
        configOutput(OUTPUT_GATE_CONNECTOR, "Gate");
        configOutput(OUTPUT_AUDIO_CONNECTOR, "Audio");
        configOutput(OUTPUT_SYNC_CONNECTOR, "Sync");
}