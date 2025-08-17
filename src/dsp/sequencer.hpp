#pragma once
#include <array>
#include <numbers>

namespace clonotribe {

struct Sequencer final {
    static constexpr int kMaxSteps = 16;
    static constexpr int kDefaultSteps = 8;
    static constexpr int kFluxBufferSize = 1600;

    struct Step {
        bool skipped = false;
        bool muted = false;
        float pitch = 0.0f;
        float gate = 0.0f;
        float gateTime = 0.5f;
        bool accent = false;
        bool glide = false;
    };

    // Accent/Glide step methods moved to Sequencer
    void setStepAccent(int step, bool value) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].accent = value;
    }
    bool isStepAccent(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].accent;
        return false;
    }
    void setStepGlide(int step, bool value) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].glide = value;
    }
    bool isStepGlide(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].glide;
        return false;
    }

    std::array<Step, kMaxSteps> steps{};
    std::array<float, kFluxBufferSize> fluxBuffer{};

    int currentStep = 0;
    int fluxRecordingStep = 0;
    int fluxSampleCount = 0;
    int recordingStep = 0;
    float fluxStepTimer = 0.0f;
    float lastRecordedPitch = 0.0f;
    float stepDuration = 0.25f;
    float stepTimer = 0.0f;
    bool externalSync = false;
    bool fluxMode = false;
    bool playing = false;
    bool recording = false;
    bool sixteenStepMode = false;

    rack::dsp::SchmittTrigger gateTrigger;
    rack::dsp::SchmittTrigger syncTrigger;

    constexpr Sequencer() noexcept = default;
    Sequencer(const Sequencer&) noexcept = default;
    Sequencer& operator=(const Sequencer&) noexcept = default;
    Sequencer(Sequencer&&) noexcept = default;
    Sequencer& operator=(Sequencer&&) noexcept = default;
    ~Sequencer() noexcept = default;

    void setTempo(float bpm) noexcept { stepDuration = 60.0f / (bpm * 4.0f); }
    void setExternalSync(bool external) noexcept { externalSync = external; }
    void setSixteenStepMode(bool sixteenStep) noexcept { sixteenStepMode = sixteenStep; }
    [[nodiscard]] bool isInSixteenStepMode() const noexcept { return sixteenStepMode; }
    int getStepCount() const noexcept { return sixteenStepMode ? 16 : 8; }
    int getStepIndex(int buttonStep, bool isSubStep = false) const noexcept {
        if (!sixteenStepMode) return buttonStep;
        return buttonStep * 2 + (isSubStep ? 1 : 0);
    }
    void setStepSkipped(int step, bool skip) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].skipped = skip;
    }
    bool isStepSkipped(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].skipped;
        return false;
    }
    void toggleStepSkipped(int step) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].skipped = !steps[step].skipped;
    }

    void setStepMuted(int step, bool mute) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].muted = mute;
    }
    bool isStepMuted(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].muted;
        return false;
    }
    void toggleStepMuted(int step) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].muted = !steps[step].muted;
    }
    void setStepGateTime(int step, float gateTime) noexcept {
        if (step >= 0 && step < getStepCount()) steps[step].gateTime = std::clamp(gateTime, 0.1f, 1.0f);
    }
    float getStepGateTime(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].gateTime;
        return 0.5f;
    }
    void play() noexcept { playing = true; currentStep = 0; stepTimer = 0.0f; }
    void stop() noexcept { playing = false; currentStep = 0; stepTimer = 0.0f; }
    void startRecording() noexcept {
        recording = true; recordingStep = 0;
        if (fluxMode) {
            fluxSampleCount = 0;
            fluxRecordingStep = 0;
            fluxStepTimer = 0.0f;
        }
    }
    void stopRecording() noexcept { recording = false; }
    void recordNote(float pitch, float gate, float gateTime = 0.5f) noexcept {
        if (recording && !fluxMode) {
            int targetStep = playing ? currentStep : recordingStep;
            if (targetStep >= 0 && targetStep < getStepCount()) {
                steps[targetStep].pitch = pitch;
                steps[targetStep].gate = gate;
                steps[targetStep].gateTime = gateTime;
                steps[targetStep].skipped = false;
                steps[targetStep].muted = false;
            }
        }
    }
    void recordNoteToStep(int step, float pitch, float gate, float gateTime = 0.5f) noexcept {
        if (recording && !fluxMode && step >= 0 && step < getStepCount()) {
            steps[step].pitch = pitch;
            steps[step].gate = gate;
            steps[step].gateTime = gateTime;
            steps[step].skipped = false;
            steps[step].muted = false;
        }
    }
    void recordFlux(float pitch) noexcept {
        if (recording && fluxMode) {
            if (playing) {
                int stepSampleIndex = currentStep * 100 + (int)((stepTimer / stepDuration) * 100);
                int maxSamples = getStepCount() * 100;
                if (stepSampleIndex >= 0 && stepSampleIndex < maxSamples && stepSampleIndex < kFluxBufferSize) {
                    if (fluxSampleCount < kFluxBufferSize) {
                        fluxBuffer[stepSampleIndex] = pitch;
                        fluxSampleCount = std::max(fluxSampleCount, stepSampleIndex + 1);
                    }
                }
            } else {
                if (fluxSampleCount < kFluxBufferSize) {
                    fluxBuffer[fluxSampleCount++] = pitch;
                }
            }
        }
    }
    struct SequencerOutput {
        float pitch = 0.0f;
        float gate = 0.0f;
        bool stepChanged = false;
        int step = 0;
    };
    SequencerOutput process(float sampleTime, float inputPitch = 0.0f, float inputGate = 0.0f, float syncSignal = 0.0f, float ribbonGateTimeMod = 0.5f) {
        SequencerOutput output;
        if (!playing) return output;
        bool wasNewStep = false;
        auto advanceToNextActiveStep = [this](int fromStep) -> int {
            int count = getStepCount();
            int next = (fromStep + 1) % count;
            for (int i = 0; i < count; ++i) {
                if (!steps[next].skipped) return next;
                next = (next + 1) % count;
            }
            return fromStep;
        };
        if (externalSync) {
            bool syncTriggered = syncTrigger.process(syncSignal > 1.0f);
            if (syncTriggered) {
                int nextStep = advanceToNextActiveStep(currentStep);
                wasNewStep = (nextStep != currentStep);
                currentStep = nextStep;
                stepTimer = 0.0f;
            }
            stepTimer += sampleTime;
        } else {
            stepTimer += sampleTime;
            if (stepTimer >= stepDuration) {
                stepTimer -= stepDuration;
                int nextStep = advanceToNextActiveStep(currentStep);
                wasNewStep = (nextStep != currentStep);
                currentStep = nextStep;
            }
        }
        output.step = currentStep;
        output.stepChanged = wasNewStep;
        if (!steps[currentStep].skipped) {
            if (steps[currentStep].muted) {
                output.pitch = 0.0f;
                output.gate = 0.0f;
            } else {
                if (fluxMode && fluxSampleCount > 0) {
                    int samplesPerStep = fluxSampleCount / getStepCount();
                    if (samplesPerStep > 0) {
                        int stepOffset = currentStep * samplesPerStep;
                        int sampleIndex = stepOffset + static_cast<int>((stepTimer / stepDuration) * static_cast<float>(samplesPerStep));
                        if (sampleIndex < fluxSampleCount) {
                            output.pitch = fluxBuffer[sampleIndex];
                        } else {
                            output.pitch = steps[currentStep].pitch;
                        }
                    } else {
                        output.pitch = steps[currentStep].pitch;
                    }
                } else {
                    output.pitch = steps[currentStep].pitch;
                }
                float effectiveGateTime = std::clamp(steps[currentStep].gateTime * ribbonGateTimeMod, 0.1f, 1.0f);
                float stepProgress = externalSync ? (stepTimer / 0.1f) : (stepTimer / stepDuration);
                output.gate = (stepProgress < effectiveGateTime) ? 5.0f : 0.0f;
            }
        } else {
            output.pitch = 0.0f;
            output.gate = 0.0f;
        }
        return output;
    }
};
}