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
#include "constants.hpp"

using namespace clonotribe;

struct Clonotribe : rack::Module {
    static float processEnvelope(Envelope::Type envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate);
    float processOutput(
        float filteredSignal, float volume, float envValue, float ribbonVolumeAutomation,
        float rhythmVolume, float sampleTime, NoiseGenerator& noiseGenerator, int currentStep, float distortion,
        float delayClock, float delayTime, float delayAmount
    );
    
    void handleSequencerAndDrumState(Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate, bool gateTriggered);

    VCO vco;
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
    MS20Filter ms20Filter;
    MS20FilterPrevious ms20FilterPrevious;
    FilterType selectedFilterType = FilterType::MS20;
    void setFilterType(FilterType type) {
        selectedFilterType = type;
        filterProcessor.setType(type);
        filterProcessor.setPointers(&ms20Filter, &ladderFilter, &moogFilter, &ms20FilterPrevious);
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

    bool stepCtrlLatch[8] = {false, false, false, false, false, false, false, false};
    float stepPrevVal[8] = {ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO};

    int selectedStepForEditing = 0;
    int syncDivideCounter = 0;

    bool drumPatterns[3][8] = {{false}};

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
        clearAllSequences();
    }

    auto readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, float, Envelope::Type, LFO::Mode, LFO::Target, LFO::Waveform, Ribbon::Mode, VCO::Waveform>;
    void updateDSPState(float volume, float rhythmVolume, float lfoIntensity, Ribbon::Mode ribbonMode, float octave, float cutoff);
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
