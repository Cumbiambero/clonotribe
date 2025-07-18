#include "sequencer_state.hpp"

SequencerState::SequencerState() {
    reset();
}

void SequencerState::reset() {
    currentStep = 0;
    for (auto& s : steps) {
        s.active = false;
        s.triggered = false;
        s.drum = Drum::None;
    }
}

void SequencerState::advance() {
    currentStep = (currentStep + 1) % NumSteps;
    for (auto& s : steps) s.triggered = false;
    if (steps[currentStep].active)
        steps[currentStep].triggered = true;
}

void SequencerState::triggerDrum(Drum drumType) {
    if (steps[currentStep].active) {
        steps[currentStep].drum = drumType;
        steps[currentStep].triggered = true;
    }
}

void SequencerState::setStepActive(int step, bool active) {
    if (step >= 0 && step < NumSteps)
        steps[step].active = active;
}

void SequencerState::setDrumForStep(int step, Drum drumType) {
    if (step >= 0 && step < NumSteps)
        steps[step].drum = drumType;
}

int SequencerState::getCurrentStep() const noexcept {
    return currentStep;
}

const SequencerState::StepState& SequencerState::getStepState(int step) const {
    return steps[step];
}
