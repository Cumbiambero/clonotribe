#pragma once
#include "plugin.hpp"
#include "dsp/dsp.hpp"
#include "ui/ui.hpp"

using namespace clonotribe;

struct Clonotribe : Module {
    static float processEnvelope(int envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate);
    static float processOutput(
        float filteredSignal, float volume, float envValue, float ribbonVolumeAutomation,
        float rhythmVolume, float sampleTime,
        KickDrum& kickDrum, SnareDrum& snareDrum, HiHat& hiHat, NoiseGenerator& noiseGenerator
    );
    void handleSequencerAndDrumState(clonotribe::Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate, bool gateTriggered);
    enum ParamId {
        PARAM_VCO_OCTAVE_KNOB,
        PARAM_VCA_LEVEL_KNOB,
        PARAM_VCO_WAVEFORM_SWITCH,
        PARAM_RHYTHM_VOLUME_KNOB,
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

    VCO vco;
    VCF vcf;
    LFO lfo;
    Envelope envelope;
    Sequencer sequencer;
    NoiseGenerator noiseGenerator;
    KickDrum kickDrum;
    SnareDrum snareDrum;
    HiHat hiHat;
    Ribbon ribbon;
    RibbonController ribbonController;
    
    dsp::SchmittTrigger gateTrigger;
    dsp::SchmittTrigger playTrigger;
    dsp::SchmittTrigger recTrigger;
    dsp::SchmittTrigger fluxTrigger;
    dsp::SchmittTrigger activeStepTrigger;
    dsp::SchmittTrigger gateTimeTrigger;    
    dsp::SchmittTrigger clearAllSequencesTrigger;
    dsp::SchmittTrigger clearSynthSequenceTrigger;
    dsp::SchmittTrigger clearDrumSequenceTrigger;
    dsp::SchmittTrigger enableAllActiveStepsTrigger;
    dsp::SchmittTrigger toggleSixteenStepModeTrigger;
    dsp::SchmittTrigger toggleLFOModeTrigger;
    dsp::SchmittTrigger gateTimesLockTrigger;
    dsp::SchmittTrigger syncHalfTempoTrigger;
    dsp::SchmittTrigger stepTriggers[8];
    dsp::SchmittTrigger drumTriggers[4];
    dsp::PulseGenerator syncPulse;

    int selectedDrumPart = 0; // 0=synth, 1=kick, 2=snare, 3=hihat
    int selectedStepForEditing = 0;
    int syncDivideCounter = 0;

    bool drumPatterns[3][8] = {{false}}; // [drum][step] - 0=kick, 1=snare, 2=hihat
    bool activeStepsSequencerSteps[16];
    bool activeStepsDrumPatterns[3][8];

    bool activeStepActive = false;    
    bool activeStepWasPressed = false;
    bool gateActive = false;
    bool gateTimeHeld = false;
    bool gateTimesLocked = false;
    bool lfoSampleAndHoldMode = false;
    bool sixteenStepModeTogglePending = false;
    bool syncHalfTempo = false;

    Clonotribe();
    void process(const ProcessArgs& args) override;

    auto readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, int, int, int, int, int, int>;
    void updateDSPState(float volume, float rhythmVolume, float lfoIntensity, int ribbonMode, float octave);
    void handleMainTriggers();
    void handleDrumSelectionAndTempo(float tempo);
    void handleSpecialGateTimeButtons(bool gateTimeHeld);
    auto processInputTriggers(float inputPitch, float gate, bool gateTimeHeld) -> std::tuple<float, float, bool, bool>;
    
private:
    void clearAllSequences();
    void clearDrumSequence();
    void clearSynthSequence();
    void enableAllActiveSteps();
    void handleActiveStep();
    void handleDrumRolls(const ProcessArgs& args, bool gateTimeHeld);
    void handleStepButtons();
    void toggleStepInCurrentMode(int step);
    void updateStepLights(const Sequencer::SequencerOutput& seqOutput);
    bool isStepActiveInCurrentMode(int step);
};
