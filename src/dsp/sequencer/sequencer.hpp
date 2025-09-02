#pragma once
#include <array>

namespace clonotribe {

enum class TempoRange {
    T10_600,
    T20_300,
    T60_180,
    SIZE
};

enum class DrumPart {
    SYNTH = 0,
    KICK = 1,
    SNARE = 2,
    HIHAT = 3
};

struct Sequencer final {
    static constexpr int MAX_STEPS = 16;
    static constexpr int DEFAULT_STEPS = 8;
    static constexpr int BUFFER_SIZE = 1600;

    struct Step {
        bool skipped = false;
        bool muted = false;
        float pitch = ZERO;
        float gate = ZERO;
        float gateTime = HALF;
        bool accent = false;
        bool glide = false;
    };

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

    std::array<Step, MAX_STEPS> steps{};
    std::array<float, BUFFER_SIZE> fluxBuffer{};

    int currentStep = 0;
    int fluxRecordingStep = 0;
    int fluxSampleCount = 0;
    int recordingStep = 0;
    float fluxStepTimer = ZERO;
    float lastRecordedPitch = ZERO;
    float stepDuration = 0.25f;
    float stepTimer = ZERO;
    float glideStatePitch = ZERO;
    bool externalSync = false;
    bool fluxMode = false;
    bool playing = false;
    bool recording = false;
    bool sixteenStepMode = false;
    bool glidePitchActive = false;

    DrumPart selectedDrumPart = DrumPart::SYNTH;

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
    
    void setSelectedDrumPart(DrumPart part) noexcept { selectedDrumPart = part; }
    [[nodiscard]] DrumPart getSelectedDrumPart() const noexcept { return selectedDrumPart; }
    
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
        if (step >= 0 && step < getStepCount()) steps[step].gateTime = std::clamp(gateTime, 0.1f, ONE);
    }
    float getStepGateTime(int step) const noexcept {
        if (step >= 0 && step < getStepCount()) return steps[step].gateTime;
        return HALF;
    }
    void play() noexcept { playing = true; currentStep = 0; stepTimer = ZERO; }
    void stop() noexcept { playing = false; currentStep = 0; stepTimer = ZERO; }
    void startRecording() noexcept {
        recording = true; recordingStep = 0;
        if (fluxMode) {
            fluxSampleCount = 0;
            fluxRecordingStep = 0;
            fluxStepTimer = ZERO;
        }
    }
    void stopRecording() noexcept { recording = false; }
    
    void clearSequence() noexcept {
        int stepCount = getStepCount();
        for (int i = 0; i < stepCount; i++) {
            steps[i].skipped = false;
            steps[i].muted = false;
            steps[i].pitch = ZERO;
            steps[i].gate = 5.0f;
            steps[i].gateTime = 0.8f;
            steps[i].accent = false;
            steps[i].glide = false;
        }
        fluxSampleCount = 0;
        fluxRecordingStep = 0;
        fluxMode = false;
        fluxStepTimer = ZERO;
        std::fill(std::begin(fluxBuffer), std::end(fluxBuffer), ZERO);
    }
    
    void enableAllSteps() noexcept {
        int stepCount = getStepCount();
        for (int i = 0; i < stepCount; i++) {
            steps[i].skipped = false;
        }
    }
    
    void recordNote(float pitch, float gate, float gateTime = HALF) noexcept {
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
    void recordNoteToStep(int step, float pitch, float gate, float gateTime = HALF) noexcept {
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
                if (stepSampleIndex >= 0 && stepSampleIndex < maxSamples && stepSampleIndex < BUFFER_SIZE) {
                    if (fluxSampleCount < BUFFER_SIZE) {
                        fluxBuffer[stepSampleIndex] = pitch;
                        fluxSampleCount = std::max(fluxSampleCount, stepSampleIndex + 1);
                    }
                }
            } else {
                if (fluxSampleCount < BUFFER_SIZE) {
                    fluxBuffer[fluxSampleCount++] = pitch;
                }
            }
        }
    }
    struct SequencerOutput {
        float pitch = ZERO;
        float gate = ZERO;
        bool stepChanged = false;
        int step = 0;
        bool accent = false;
        bool glide = false;
    };
    SequencerOutput process(float sampleTime, float inputPitch = ZERO, float inputGate = ZERO, float syncSignal = ZERO, float ribbonGateTimeMod = HALF, float accentGlideAmount = ZERO) {
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
            bool syncTriggered = syncTrigger.process(syncSignal > ONE);
            if (syncTriggered) {
                int nextStep = advanceToNextActiveStep(currentStep);
                wasNewStep = (nextStep != currentStep);
                currentStep = nextStep;
                stepTimer = ZERO;
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
                output.pitch = ZERO;
                output.gate = ZERO;
            } else {
                output.accent = steps[currentStep].accent;
                output.glide = steps[currentStep].glide;
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
                
                if (output.glide && accentGlideAmount > ZERO) {
                    if (!glidePitchActive) {
                        glideStatePitch = output.pitch;
                        glidePitchActive = true;
                    } else {
                        float glideSpeed = std::clamp(accentGlideAmount, ZERO, ONE);
                        glideStatePitch += (output.pitch - glideStatePitch) * glideSpeed;
                    }
                    
                    output.pitch = glideStatePitch;
                } else {
                    glideStatePitch = output.pitch;
                    glidePitchActive = true;
                }
                
                float effectiveGateTime = std::clamp(steps[currentStep].gateTime * ribbonGateTimeMod, 0.1f, ONE);
                float stepProgress = externalSync ? (stepTimer / 0.1f) : (stepTimer / stepDuration);
                output.gate = (stepProgress < effectiveGateTime) ? 5.0f : ZERO;
            }
        } else {
            output.pitch = ZERO;
            output.gate = ZERO;
            output.accent = false;
            output.glide = false;
        }
        return output;
    }
};
}