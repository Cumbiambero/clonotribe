#include "../clonotribe.hpp"

void Clonotribe::handleSequencerAndDrumState(clonotribe::Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate, bool gateTriggered) {
    if (sequencer.playing) {
        int currentStep = seqOutput.step;
        if (sequencer.isStepMuted(currentStep)) {
            seqOutput.gate = 0.0f;
            seqOutput.pitch = 0.0f;
        }
    }

    if (sequencer.recording) {
        if (sequencer.fluxMode) {
            if (finalGate > 1.0f) {
                sequencer.recordFlux(finalInputPitch);
            }
        } else if(gateTriggered) {
            if (sequencer.playing) {
                sequencer.recordNote(finalInputPitch, finalGate > 1.0f ? finalGate : 5.0f, 0.8f);
            } else {
                int stepCount = sequencer.getStepCount();
                int nextStep = sequencer.recordingStep;
                for (int i = 0; i < stepCount; ++i) {
                    if (!sequencer.isStepSkipped(nextStep)) break;
                    nextStep = (nextStep + 1) % stepCount;
                }
                bool allSkipped = true;
                for (int i = 0; i < stepCount; ++i) {
                    if (!sequencer.isStepSkipped(i)) { allSkipped = false; break; }
                }
                if (allSkipped) {
                    sequencer.setStepSkipped(0, false);
                    nextStep = 0;
                }
                sequencer.recordNoteToStep(nextStep, finalInputPitch, finalGate > 1.0f ? finalGate : 5.0f, 0.8f);
                sequencer.recordingStep = (nextStep + 1) % stepCount;
            }
        }
    }

    // Trigger drums on step changes and generate sync pulses
    if (sequencer.playing && seqOutput.stepChanged) {
        int currentStep = seqOutput.step;

        int drumStepIndex = currentStep;
        if (sequencer.isInSixteenStepMode()) {
            // In 16-step mode, drums trigger on main steps (even indices 0,2,4,6,8,10,12,14) map these back to 0,1,2,3,4,5,6,7
            if (currentStep % 2 == 0 && currentStep < 16) {
                drumStepIndex = currentStep / 2;
            } else {
                drumStepIndex = -1; // don't trigger drums on sub-steps
            }
        }

        if (drumStepIndex >= 0 && drumStepIndex < 8 && !sequencer.isStepSkipped(currentStep)) {
            if (drumPatterns[0][drumStepIndex]) triggerKick();
            if (drumPatterns[1][drumStepIndex]) triggerSnare();
            if (drumPatterns[2][drumStepIndex]) triggerHihat();
        }
        syncPulse.trigger(1e-3f); // Generate sync pulse on step change (1ms pulse)
    }
}
