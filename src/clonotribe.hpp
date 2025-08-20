
#pragma once
#include <rack.hpp>

#include "plugin.hpp"
#include "dsp/dsp.hpp"
#include "dsp/parameter_cache.hpp"
#include "dsp/drumkits/base/drum_processor.hpp"
#include "dsp/filter_processor.hpp"
#include "dsp/vcf/ladder.hpp"
#include "dsp/vcf/filter_type.hpp"
#include "dsp/delay.hpp"
#include "dsp/dc_blocker.hpp"
#include "ui/ui.hpp"

using namespace clonotribe;

struct Clonotribe : rack::Module {
    static float processEnvelope(Envelope::Type envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate);
    float processOutput(
        float filteredSignal, float volume, float envValue, float ribbonVolumeAutomation,
        float rhythmVolume, float sampleTime, NoiseGenerator& noiseGenerator, int currentStep, float distortion,
        float delayClock, float delayTime, float delayAmount
    );
    
    void handleSequencerAndDrumState(Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate, bool gateTriggered);
    enum ParamId {
        PARAM_VCO_OCTAVE_KNOB,
        PARAM_VCA_LEVEL_KNOB,
        PARAM_VCO_WAVEFORM_SWITCH,
        PARAM_RHYTHM_VOLUME_KNOB,
        PARAM_DISTORTION_KNOB,
        PARAM_DELAY_TIME_KNOB,
        PARAM_DELAY_AMOUNT_KNOB,
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
        PARAM_ACCENT_GLIDE_KNOB,
        PARAMS_LEN
    };
    
    enum InputId {
        INPUT_CV_CONNECTOR,
        INPUT_GATE_CONNECTOR,
        INPUT_AUDIO_CONNECTOR,
        INPUT_SYNC_CONNECTOR,
        INPUT_VCO_OCTAVE_CONNECTOR,
        INPUT_VCF_CUTOFF_CONNECTOR,
        INPUT_VCF_PEAK_CONNECTOR,
        INPUT_VCA_CONNECTOR,
        INPUT_LFO_RATE_CONNECTOR,
        INPUT_LFO_INTENSITY_CONNECTOR,
        INPUT_DISTORTION_CONNECTOR,
        INPUT_DELAY_TIME_CONNECTOR,
        INPUT_DELAY_AMOUNT_CONNECTOR,
        INPUT_NOISE_CONNECTOR,
        INPUT_ACCENT_GLIDE_CONNECTOR,
        INPUTS_LEN
    };
    
    enum OutputId {
        OUTPUT_CV_CONNECTOR,
        OUTPUT_GATE_CONNECTOR,
        OUTPUT_AUDIO_CONNECTOR,
        OUTPUT_SYNC_CONNECTOR,
        OUTPUT_SYNTH_CONNECTOR,
        OUTPUT_BASSDRUM_CONNECTOR,
        OUTPUT_SNARE_CONNECTOR,
        OUTPUT_HIHAT_CONNECTOR,
        OUTPUT_LFO_RATE_CONNECTOR,        
        OUTPUTS_LEN
    };
    
    enum LightId {
        LIGHT_SYNTH,
        LIGHT_BASSDRUM,
        LIGHT_SNARE,
        LIGHT_HIGHHAT,
        LIGHT_FLUX,
        LIGHT_REC,
        LIGHT_PLAY,
        LIGHT_SEQUENCER_1_R,
        LIGHT_SEQUENCER_1_G,
        LIGHT_SEQUENCER_1_B,
        LIGHT_SEQUENCER_2_R,
        LIGHT_SEQUENCER_2_G,
        LIGHT_SEQUENCER_2_B,
        LIGHT_SEQUENCER_3_R,
        LIGHT_SEQUENCER_3_G,
        LIGHT_SEQUENCER_3_B,
        LIGHT_SEQUENCER_4_R,
        LIGHT_SEQUENCER_4_G,
        LIGHT_SEQUENCER_4_B,
        LIGHT_SEQUENCER_5_R,
        LIGHT_SEQUENCER_5_G,
        LIGHT_SEQUENCER_5_B,
        LIGHT_SEQUENCER_6_R,
        LIGHT_SEQUENCER_6_G,
        LIGHT_SEQUENCER_6_B,
        LIGHT_SEQUENCER_7_R,
        LIGHT_SEQUENCER_7_G,
        LIGHT_SEQUENCER_7_B,
        LIGHT_SEQUENCER_8_R,
        LIGHT_SEQUENCER_8_G,
        LIGHT_SEQUENCER_8_B,
        LIGHTS_LEN
    };

    VCO vco;
    MS20Filter vcf;
    LFO lfo;
    Envelope envelope;
    Sequencer sequencer;
    NoiseGenerator noiseGenerator;
    
    DrumProcessor drumProcessor;
    Distortion distortionProcessor;
    Delay delayProcessor;
    DrumKitType selectedDrumKit = DrumKitType::ORIGINAL;
    NoiseType selectedNoiseType = NoiseType::WHITE;

    void setDrumKit(DrumKitType kit) {
        selectedDrumKit = kit;
        drumProcessor.setDrumKit(static_cast<DrumKitType>(kit));
    }
    
    void setNoiseType(NoiseType type) {
        selectedNoiseType = type;
        noiseGenerator.setNoiseType(type);
    }
    
    void triggerKick() { drumProcessor.triggerKick(); }
    void triggerSnare() { drumProcessor.triggerSnare(); }
    void triggerHihat() { drumProcessor.triggerHihat(); }
    Ribbon ribbon;
    FilterProcessor filterProcessor;
    LadderFilter ladderFilter;
    MoogFilter moogFilter;
    FilterType selectedFilterType = FilterType::MS20;
    void setFilterType(FilterType type) {
        selectedFilterType = type;
        filterProcessor.setType(type);
        filterProcessor.setPointers(&vcf, &ladderFilter, &moogFilter);
    }
    RibbonController ribbonController;
    
    ParameterCache paramCache;
    
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
    dsp::SchmittTrigger ribbonGateTrigger;
    dsp::PulseGenerator syncPulse;
    dsp::PulseGenerator lfoRatePulse;
    
    DcBlocker dcBlockerPost;
    DcBlocker dcBlockerPostFilter;
    DcBlocker dcBlockerPostDist;
    DcBlocker dcBlockerFinal;
    
    bool stepCtrlLatch[8] = {false,false,false,false,false,false,false,false};
    float stepPrevVal[8] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

    int selectedStepForEditing = 0;
    int syncDivideCounter = 0;

    bool drumPatterns[3][8] = {{false}}; // [drum][step] - 0=kick, 1=snare, 2=hihat

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
    void processBypass(const ProcessArgs& args) override;
    void onRandomize(const RandomizeEvent& e) override;
    
    void onSampleRateChange() override {
        drumProcessor.setSampleRate(APP->engine->getSampleRate());
        delayProcessor.setSampleRate(APP->engine->getSampleRate());
        
        float sampleRate = APP->engine->getSampleRate();
        dcBlockerPost.setSampleRate(sampleRate);
        dcBlockerPost.setCutoff(30.0f);
        
        dcBlockerPostFilter.setSampleRate(sampleRate);
        dcBlockerPostFilter.setCutoff(20.0f);
        
        dcBlockerPostDist.setSampleRate(sampleRate);
        dcBlockerPostDist.setCutoff(15.0f);
        
        dcBlockerFinal.setSampleRate(sampleRate);
        dcBlockerFinal.setCutoff(10.0f);
    }

    void onReset() override {
        Module::onReset();
        delayProcessor.clear();
    }

    auto readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, float, Envelope::Type, LFO::Mode, LFO::Target, LFO::Waveform, Ribbon::Mode, VCO::Waveform>;
    void updateDSPState(float volume, float rhythmVolume, float lfoIntensity, Ribbon::Mode ribbonMode, float octave);
    void handleMainTriggers();
    void handleDrumSelectionAndTempo(float tempo);
    void handleSpecialGateTimeButtons(bool gateTimeHeld);
    auto processInputTriggers(float inputPitch, float gate, bool gateTimeHeld) -> std::tuple<float, float, bool, bool>;
    
    void appendContextMenu(rack::ui::Menu* menu);
    
    json_t* dataToJson() override;
    void dataFromJson(json_t* rootJ) override;

public:
    TempoRange selectedTempoRange = TempoRange::T10_600;

    void getTempoRange(float& min, float& max) {
        switch (selectedTempoRange) {
            case TempoRange::T10_600: min = 10.0f; max = 600.0f; break;
            case TempoRange::T20_300: min = 20.0f; max = 300.0f; break;
            case TempoRange::T60_180: min = 60.0f; max = 180.0f; break;
            default: min = 10.0f; max = 600.0f;
        }
    }

private:
    void clearAllSequences();
    void clearDrumSequence();
    void clearSynthSequence();
    void enableAllActiveSteps();
    void handleActiveStep();
    void handleDrumRolls(const ProcessArgs& args, bool gateTimeHeld);
    void handleStepButtons(float sampleTime);
    void toggleStepInCurrentMode(int step);
    void toggleActiveStep(int step);
    void updateStepLights(const Sequencer::SequencerOutput& seqOutput);
    bool isStepActiveInCurrentMode(int step);
    void cycleStepAccentGlide(int step);
};
