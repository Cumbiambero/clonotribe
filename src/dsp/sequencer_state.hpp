#pragma once
#include <array>
#include <cstdint>

class SequencerState final {
public:
    static constexpr int NumSteps = 8;
    enum class Drum { Synth, BassDrum, Snare, HiHat, None };

    struct StepState {
        bool active = false;
        bool triggered = false;
        Drum drum = Drum::None;
    };

    SequencerState();
    void reset();
    void advance();
    void triggerDrum(Drum drumType);
    void setStepActive(int step, bool active);
    void setDrumForStep(int step, Drum drumType);
    int getCurrentStep() const noexcept;
    const StepState& getStepState(int step) const;

private:
    int currentStep = 0;
    std::array<StepState, NumSteps> steps;
};
