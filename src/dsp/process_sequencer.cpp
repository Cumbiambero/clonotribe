#include "../clonotribe.hpp"

void Clonotribe::handleSequencerAndDrumState(clonotribe::Sequencer::SequencerOutput& seqOutput, float finalInputPitch, float finalGate, bool gateTriggered) {
    // Override sequencer output with active steps when ACTIVE STEP is active
    if (activeStepActive && sequencer.playing) {
        int currentStep = seqOutput.step;
        if (currentStep >= 0 && currentStep < sequencer.getStepCount()) {
            if (selectedDrumPart == 0) {
                if (!activeStepsSequencerSteps[currentStep]) {
                    seqOutput.gate = 0.0f;
                }
            }
        }
    }

    if (sequencer.recording && selectedDrumPart == 0) {
        if (sequencer.fluxMode) {
            if (finalGate > 1.0f) {
                sequencer.recordFlux(finalInputPitch);
            }
        } else {
            if (sequencer.playing) {
                // Record on any gate trigger (CV/Gate input OR ribbon touch)
                if (gateTriggered) {
                    sequencer.recordNote(finalInputPitch, finalGate > 1.0f ? finalGate : 5.0f, 0.8f);
                }
            } else {
                // When not playing, record on gate trigger from any source
                if (gateTriggered) {
                    sequencer.recordNoteToStep(sequencer.recordingStep, finalInputPitch, finalGate > 1.0f ? finalGate : 5.0f, 0.8f);
                    sequencer.recordingStep = (sequencer.recordingStep + 1) % sequencer.getStepCount();
                }
            }
        }
    }

    // Trigger drums on step changes and generate sync pulses
    if (sequencer.playing && seqOutput.stepChanged) {
        int currentStep = seqOutput.step;

        // Drums only use first 8 steps, even in 16-step mode
        int drumStepIndex = currentStep;
        if (sequencer.isInSixteenStepMode()) {
            // In 16-step mode, drums trigger on main steps (even indices 0,2,4,6,8,10,12,14) map these back to 0,1,2,3,4,5,6,7
            if (currentStep % 2 == 0 && currentStep < 16) {
                drumStepIndex = currentStep / 2;
            } else {
                drumStepIndex = -1; // don't trigger drums on sub-steps
            }
        }

        if (drumStepIndex >= 0 && drumStepIndex < 8) {
            if (activeStepActive) {
                if (activeStepsDrumPatterns[0][drumStepIndex]) kickDrum.trigger();
                if (activeStepsDrumPatterns[1][drumStepIndex]) snareDrum.trigger();
                if (activeStepsDrumPatterns[2][drumStepIndex]) hiHat.trigger();
            } else {
                if (drumPatterns[0][drumStepIndex]) kickDrum.trigger();
                if (drumPatterns[1][drumStepIndex]) snareDrum.trigger();
                if (drumPatterns[2][drumStepIndex]) hiHat.trigger();
            }
        }
        syncPulse.trigger(1e-3f); // Generate sync pulse on step change (1ms pulse)
    }
}
