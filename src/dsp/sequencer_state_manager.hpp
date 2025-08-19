#pragma once
#include "sequencer.hpp"

namespace clonotribe {

class SequencerStateManager {
public:
    enum class LFOMode { OneShot = 0, Slow = 1, Fast = 2 };
    enum class LFOTarget { VCF = 0, VCO_VCF = 1, VCO = 2 };
    enum class LFOWaveform { Square = 0, Triangle = 1, Sawtooth = 2 };

    struct TriggerState {
        bool playPressed = false;
        bool recPressed = false;
        bool fluxPressed = false;
        bool activeStepPressed = false;
        bool gateTimePressed = false;
        bool stepPressed[8] = {false};
        bool drumPressed[4] = {false};
    };
    
    struct UIState {
        int selectedDrumPart = 0;
        int selectedStepForEditing = 0;
        bool activeStepActive = false;
        bool activeStepWasPressed = false;
        bool gateActive = false;
        bool gateTimeHeld = false;
        bool gateTimesLocked = false;
        bool lfoSampleAndHoldMode = false;
        bool sixteenStepModeTogglePending = false;
        bool syncHalfTempo = false;
        int syncDivideCounter = 0;
    };
    
    void updateTriggers(const float* params);
    void handlePlaybackControls(Sequencer& sequencer, const TriggerState& triggers);
    void handleStepControls(Sequencer& sequencer, UIState& uiState, const TriggerState& triggers);
    void handleDrumSelection(UIState& uiState, const TriggerState& triggers);
    void updateLights(const Sequencer& sequencer, const UIState& uiState, float* lights);

private:
    rack::dsp::SchmittTrigger playTrigger;
    rack::dsp::SchmittTrigger recTrigger;
    rack::dsp::SchmittTrigger fluxTrigger;
    rack::dsp::SchmittTrigger activeStepTrigger;
    rack::dsp::SchmittTrigger gateTimeTrigger;
    rack::dsp::SchmittTrigger stepTriggers[8];
    rack::dsp::SchmittTrigger drumTriggers[4];
    
    rack::dsp::SchmittTrigger clearAllSequencesTrigger;
    rack::dsp::SchmittTrigger clearSynthSequenceTrigger;
    rack::dsp::SchmittTrigger clearDrumSequenceTrigger;
    rack::dsp::SchmittTrigger enableAllActiveStepsTrigger;
    rack::dsp::SchmittTrigger toggleSixteenStepModeTrigger;
    rack::dsp::SchmittTrigger toggleLFOModeTrigger;
    rack::dsp::SchmittTrigger gateTimesLockTrigger;
    rack::dsp::SchmittTrigger syncHalfTempoTrigger;
};

}
