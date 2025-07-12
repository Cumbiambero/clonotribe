#pragma once
#include "plugin.hpp"
#include "dsp.hpp"
#include <random>

struct Clonotribe : Module {
    enum ParamId {
        PARAM_VCO_OCTAVE_KNOB,
        PARAM_VCA_LEVEL_KNOB,
        PARAM_VCO_WAVEFORM_SWITCH,
        PARAM_RHYTM_VOLUME_KNOB,
        PARAM_SNARE_BUTTON,
        PARAM_FLUX_BUTTON,
        PARAM_REC_BUTTON,
        PARAM_VCF_CUTOFF_KNOB,
        PARAM_LFO_RATE_KNOB,
        PARAM_LFO_INTERVAL_KNOB,
        PARAM_NOISE_KNOB,
        PARAM_VCF_PEAK_KNOB,
        PARAM_RIBBON_RANGE_SWITCH,
        PARAM_ENVELOPE_FORM_SWITCH,
        PARAM_LFO_TARGET_SWITCH,
        PARAM_LFO_MODE_SWITCH,
        PARAM_LFO_WAVEFORM_SWITCH,
        PARAM_SEQUENCER_TEMPO_KNOB,
        PARAM_SYNTH_BUTTON,
        PARAM_BASSDRUM_BUTTON,
        PARAM_HIGHHAT_BUTTON,
        PARAM_ACTIVE_STEP_BUTTON,
        PARAM_SEQUENCER_1_BUTTON,
        PARAM_SEQUENCER_2_BUTTON,
        PARAM_SEQUENCER_3_BUTTON,
        PARAM_SEQUENCER_4_BUTTON,
        PARAM_SEQUENCER_5_BUTTON,
        PARAM_SEQUENCER_6_BUTTON,
        PARAM_SEQUENCER_7_BUTTON,
        PARAM_SEQUENCER_8_BUTTON,
        PARAM_PLAY_BUTTON,
        PARAM_GATE_TIME_BUTTON,
        PARAMS_LEN
    };
    
    enum InputId {
        INPUT_CV_CONNECTOR,
        INPUT_GATE_CONNECTOR,
        INPUT_AUDIO_CONNECTOR,
        INPUT_SYNC_CONNECTOR,
        INPUTS_LEN
    };
    
    enum OutputId {
        OUTPUT_CV_CONNECTOR,
        OUTPUT_GATE_CONNECTOR,
        OUTPUT_AUDIO_CONNECTOR,
        OUTPUT_SYNC_CONNECTOR,
        OUTPUTS_LEN
    };
    
    enum LightId {
        LIGHT_SYNTH,
        LIGHT_BASSDRUM,
        LIGHT_SNARE,
        LIGHT_HIGHHAT,
        LIGHT_SEQUENCER_1,
        LIGHT_SEQUENCER_2,
        LIGHT_SEQUENCER_3,
        LIGHT_SEQUENCER_4,
        LIGHT_SEQUENCER_5,
        LIGHT_SEQUENCER_6,
        LIGHT_SEQUENCER_7,
        LIGHT_SEQUENCER_8,
        LIGHT_FLUX,
        LIGHT_REC,
        LIGHT_PLAY,
        LIGHTS_LEN
    };

    // DSP components
    clonotribe::VCO vco;
    clonotribe::MS20Filter vcf;
    clonotribe::LFO lfo;
    clonotribe::ADSR envelope;
    clonotribe::Sequencer sequencer;
    
    // Shared noise generator for performance
    clonotribe::NoiseGenerator noiseGen;
    
    // Drum components
    clonotribe::KickDrum kickDrum;
    clonotribe::SnareDrum snareDrum;
    clonotribe::HiHat hiHat;
    clonotribe::RibbonController ribbon;
    
    // Gate detection
    dsp::SchmittTrigger gateTrigger;
    dsp::SchmittTrigger playTrigger;
    dsp::SchmittTrigger recTrigger;
    dsp::SchmittTrigger fluxTrigger;
    dsp::SchmittTrigger stepTriggers[8];
    bool gateActive = false;
    
    // Drum pattern storage
    bool drumPatterns[3][8] = {{false}}; // [drum][step] - 0=kick, 1=snare, 2=hihat
    int selectedDrumPart = 0; // 0=synth, 1=kick, 2=snare, 3=hihat
    dsp::SchmittTrigger drumTriggers[4]; // For the 4 part selection buttons
    
    // Step editing
    int selectedStepForEditing = 0; // Which step is selected for ACTIVE STEP/GATE TIME editing
    dsp::SchmittTrigger activeStepTrigger;
    dsp::SchmittTrigger gateTimeTrigger;
    
    // Sync output generation
    dsp::PulseGenerator syncPulse;
    
    std::default_random_engine noiseEngine;
    std::uniform_real_distribution<float> noiseDist{-1.f, 1.f};

    Clonotribe();
    void process(const ProcessArgs& args) override;
};
