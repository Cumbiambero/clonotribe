#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

struct Sequencer {
    struct Step {
        bool active = true;
        float pitch = 0.f;
        float gate = 0.f;
        float gateTime = 0.5f; // 0.0 to 1.0
    };
    
    Step steps[16];
    int currentStep = 0;
    float stepTimer = 0.f;
    float stepDuration = 0.25f; // Quarter note at 120 BPM
    bool playing = false;
    bool recording = false;
    bool fluxMode = false;
    bool externalSync = false;
    bool sixteenStepMode = false;
    int recordingStep = 0;
    
    dsp::SchmittTrigger syncTrigger;
    dsp::SchmittTrigger gateTrigger; 
    
    float lastRecordedPitch = 0.f;
    float fluxBuffer[1600];
    int fluxSampleCount = 0;
    int fluxRecordingStep = 0;
    float fluxStepTimer = 0.f;
    
    void setTempo(float bpm) {
        stepDuration = 60.f / (bpm * 4.f); // 16th notes
    }
    
    void setExternalSync(bool external) {
        externalSync = external;
    }
    
    void setSixteenStepMode(bool sixteenStep) {
        sixteenStepMode = sixteenStep;
    }
    
    bool isSixteenStepMode() const {
        return sixteenStepMode;
    }
    
    int getStepCount() const {
        return sixteenStepMode ? 16 : 8;
    }
    

    int getStepIndex(int buttonStep, bool isSubStep = false) const {
        if (!sixteenStepMode) {
            return buttonStep;
        }
        return buttonStep * 2 + (isSubStep ? 1 : 0);
    }
    
    void setStepActive(int step, bool active) {
        if (step >= 0 && step < getStepCount()) {
            steps[step].active = active;
        }
    }
    
    bool isStepActive(int step) const {
        if (step >= 0 && step < getStepCount()) {
            return steps[step].active;
        }
        return false;
    }
    
    void setStepGateTime(int step, float gateTime) {
        if (step >= 0 && step < getStepCount()) {
            steps[step].gateTime = clamp(gateTime, 0.1f, 1.0f);
        }
    }
    
    float getStepGateTime(int step) const {
        if (step >= 0 && step < getStepCount()) {
            return steps[step].gateTime;
        }
        return 0.5f;
    }
    
    void play() {
        playing = true;
        currentStep = 0;
        stepTimer = 0.f;
    }
    
    void stop() {
        playing = false;
        currentStep = 0;
        stepTimer = 0.f;
    }
    
    void startRecording() {
        recording = true;
        recordingStep = 0;
        if (fluxMode) {
            fluxSampleCount = 0;
            fluxRecordingStep = 0;
            fluxStepTimer = 0.f;
        }
    }
    
    void stopRecording() {
        recording = false;
    }
    
    void recordNote(float pitch, float gate, float gateTime = 0.5f) {
        if (recording && !fluxMode) {
            int targetStep = playing ? currentStep : recordingStep;
            if (targetStep >= 0 && targetStep < getStepCount()) {
                steps[targetStep].pitch = pitch;
                steps[targetStep].gate = gate;
                steps[targetStep].gateTime = gateTime;
                steps[targetStep].active = true;
            }
        }
    }
    
    void recordNoteToStep(int step, float pitch, float gate, float gateTime = 0.5f) {
        if (recording && !fluxMode && step >= 0 && step < getStepCount()) {
            steps[step].pitch = pitch;
            steps[step].gate = gate;
            steps[step].gateTime = gateTime;
            steps[step].active = true;
        }
    }
    
    void recordFlux(float pitch) {
        if (recording && fluxMode) {
            if (playing) {
                int stepSampleIndex = currentStep * 100 + (int)((stepTimer / stepDuration) * 100);
                int maxSamples = getStepCount() * 100;
                if (stepSampleIndex >= 0 && stepSampleIndex < maxSamples && stepSampleIndex < 1600) {
                    if (fluxSampleCount < 1600) {
                        fluxBuffer[stepSampleIndex] = pitch;
                        fluxSampleCount = std::max(fluxSampleCount, stepSampleIndex + 1);
                    }
                }
            } else {
                if (fluxSampleCount < 1600) {
                    fluxBuffer[fluxSampleCount++] = pitch;
                }
            }
        }
    }
    
    struct SequencerOutput {
        float pitch = 0.f;
        float gate = 0.f;
        bool stepChanged = false;
        int step = 0;
    };
    
    SequencerOutput process(float sampleTime, float inputPitch = 0.f, float inputGate = 0.f, float syncSignal = 0.f, float ribbonGateTimeMod = 0.5f) {
        SequencerOutput output;
        
        if (!playing) {
            return output;
        }
        
        bool wasNewStep = false;
        
        if (externalSync) {
            bool syncTriggered = syncTrigger.process(syncSignal > 1.f);
            if (syncTriggered) {
                currentStep = (currentStep + 1) % getStepCount();
                wasNewStep = true;
                stepTimer = 0.f;
            }
            stepTimer += sampleTime;
        } else {
            stepTimer += sampleTime;
            if (stepTimer >= stepDuration) {
                stepTimer -= stepDuration;
                currentStep = (currentStep + 1) % getStepCount();
                wasNewStep = true;
            }
        }
        
        output.step = currentStep;
        output.stepChanged = wasNewStep;
        
        // Always process the current step, but output silence if inactive
        if (steps[currentStep].active) {
            if (fluxMode && fluxSampleCount > 0) {
                int samplesPerStep = fluxSampleCount / getStepCount();
                if (samplesPerStep > 0) {
                    int stepOffset = currentStep * samplesPerStep;
                    int sampleIndex = stepOffset + (int)((stepTimer / stepDuration) * samplesPerStep);
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
            
            float effectiveGateTime = clamp(steps[currentStep].gateTime * ribbonGateTimeMod, 0.1f, 1.0f);
            float stepProgress = externalSync ? (stepTimer / 0.1f) : (stepTimer / stepDuration); 
            output.gate = (stepProgress < effectiveGateTime) ? 5.f : 0.f;
        } else {
            output.pitch = 0.f;
            output.gate = 0.f;
        }
        return output;
    }
};

}
