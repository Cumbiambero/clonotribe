#include "../../clonotribe.hpp"

void Clonotribe::handleSequencerAndDrumState(clonotribe::Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate,bool gateTriggered) {
    if (sequencer.playing) {
        int currentStep = seqOutput.step;
        if (sequencer.isStepMuted(currentStep)) {
            seqOutput.gate = OFF;
            seqOutput.pitch = OFF;
        }
    }

    if (sequencer.recording) {
        if (sequencer.fluxMode) {
            if (finalGate > ONE) {
                sequencer.recordFlux(finalInputPitch);
            }
        } else if(gateTriggered) {
            if (sequencer.playing) {
                sequencer.recordNote(finalInputPitch, finalGate > ONE ? finalGate : 5.0f, 0.8f);
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
                sequencer.recordNoteToStep(nextStep, finalInputPitch, finalGate > ONE ? finalGate : 5.0f, 0.8f);
                sequencer.recordingStep = (nextStep + 1) % stepCount;
            }
        } else if (sequencer.playing && ribbon.touching && seqOutput.stepChanged) {
            sequencer.recordNote(finalInputPitch, finalGate > ONE ? finalGate : 5.0f, 0.8f);
        }
    }

    if (sequencer.playing && seqOutput.stepChanged) {
        int currentStep = seqOutput.step;

        int drumStepIndex = currentStep;
        if (sequencer.isInSixteenStepMode()) {
            if (currentStep % 2 == 0 && currentStep < 16) {
                drumStepIndex = currentStep >> 1;
            } else {
                drumStepIndex = -1;
            }
        }

        if (drumStepIndex >= 0 && drumStepIndex < 8 && !sequencer.isStepSkipped(currentStep)) {
            if (drumPatterns[0][drumStepIndex]) triggerKick();
            if (drumPatterns[1][drumStepIndex]) triggerSnare();
            if (drumPatterns[2][drumStepIndex]) triggerHihat();
        }
        syncPulse.trigger(1e-3f);
    }
}
