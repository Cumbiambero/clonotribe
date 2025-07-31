#include "../clonotribe.hpp"
#include <tuple>

auto Clonotribe::readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, int, int, int, int, int, int> {
    float cutoff = params[PARAM_VCF_CUTOFF_KNOB].getValue();
    float lfoIntensity = params[PARAM_LFO_INTERVAL_KNOB].getValue();
    float lfoRate = params[PARAM_LFO_RATE_KNOB].getValue();
    float noiseLevel = params[PARAM_NOISE_KNOB].getValue();
    float resonance = params[PARAM_VCF_PEAK_KNOB].getValue();
    float rhythmVolume = params[PARAM_RHYTHM_VOLUME_KNOB].getValue();
    float tempo = params[PARAM_SEQUENCER_TEMPO_KNOB].getValue();
    float volume = params[PARAM_VCA_LEVEL_KNOB].getValue();
    float octaveSwitch = (float) params[PARAM_VCO_OCTAVE_KNOB].getValue();
    float octave = octaveSwitch - 3.0f;
    int envelopeType = (int) params[PARAM_ENVELOPE_FORM_SWITCH].getValue();
    int lfoMode = (int) params[PARAM_LFO_MODE_SWITCH].getValue();
    int lfoTarget = (int) params[PARAM_LFO_TARGET_SWITCH].getValue();
    int lfoWaveform = (int) params[PARAM_LFO_WAVEFORM_SWITCH].getValue();
    int ribbonMode = (int) params[PARAM_RIBBON_RANGE_SWITCH].getValue();
    int waveform = (int) params[PARAM_VCO_WAVEFORM_SWITCH].getValue();
    return {cutoff, lfoIntensity, lfoRate, noiseLevel, resonance, rhythmVolume, tempo, volume, octave, envelopeType, lfoMode, lfoTarget, lfoWaveform, ribbonMode, waveform};
}

void Clonotribe::updateDSPState(float volume, float rhythmVolume, float lfoIntensity, int ribbonMode, float octave) {
    bool synthActive = (selectedDrumPart == 0) || (volume > 0.01f && rhythmVolume < 0.99f);
    bool filterActive = synthActive;
    bool lfoActive = synthActive && (lfoIntensity > 0.01f);
    lfo.active = lfoActive;
    vco.active = synthActive;
    vcf.setActive(filterActive);
    ribbon.setMode(ribbonMode);
    ribbon.setOctave(octave);
}

void Clonotribe::handleMainTriggers() {
    bool playPressed = playTrigger.process(params[PARAM_PLAY_BUTTON].getValue() > 0.5f);
    bool recPressed = recTrigger.process(params[PARAM_REC_BUTTON].getValue() > 0.5f);
    bool fluxPressed = fluxTrigger.process(params[PARAM_FLUX_BUTTON].getValue() > 0.5f);
    if (playPressed) {
        if (sequencer.playing) {
            sequencer.stop();
        } else {
            sequencer.play();
        }
    }
    if (recPressed) {
        if (params[PARAM_PLAY_BUTTON].getValue() > 0.5f) {
            if (!sequencer.recording) {
                sequencer.startRecording();
                if (!sequencer.playing) {
                    sequencer.play();
                }
            }
        } else {
            if (sequencer.recording) {
                sequencer.stopRecording();
            } else {
                sequencer.startRecording();
            }
        }
    }
    if (fluxPressed) {
        sequencer.fluxMode = !sequencer.fluxMode;
    }
}

void Clonotribe::handleDrumSelectionAndTempo(float tempo) {
    bool synthPressed = drumTriggers[0].process(params[PARAM_SYNTH_BUTTON].getValue() > 0.5f);
    bool kickPressed = drumTriggers[1].process(params[PARAM_BASSDRUM_BUTTON].getValue() > 0.5f);
    bool snarePressed = drumTriggers[2].process(params[PARAM_SNARE_BUTTON].getValue() > 0.5f);
    bool hihatPressed = drumTriggers[3].process(params[PARAM_HIGHHAT_BUTTON].getValue() > 0.5f);
    if (synthPressed) selectedDrumPart = 0;
    if (kickPressed) selectedDrumPart = 1;
    if (snarePressed) selectedDrumPart = 2;
    if (hihatPressed) selectedDrumPart = 3;
    if (!inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
        float minTempo, maxTempo;
        getTempoRange(minTempo, maxTempo);
        float bpm = rack::math::rescale(tempo, 0.0f, 1.0f, minTempo, maxTempo);
        sequencer.setTempo(bpm);
        sequencer.setExternalSync(false);
    } else {
        sequencer.setExternalSync(true);
    }
}

void Clonotribe::handleSpecialGateTimeButtons(bool gateTimeHeld) {
    if (gateTimeHeld) {
        bool seq6Pressed = toggleSixteenStepModeTrigger.process(params[PARAM_SEQUENCER_6_BUTTON].getValue() > 0.5f);
        if (seq6Pressed) {
            sequencer.setSixteenStepMode(!sequencer.isInSixteenStepMode());
            if (sequencer.currentStep >= sequencer.getStepCount()) {
                sequencer.currentStep = 0;
            }
        }
        bool clearAllSequencesRequested = clearAllSequencesTrigger.process(params[PARAM_SEQUENCER_1_BUTTON].getValue() > 0.5f);
        bool clearSynthSequenceRequested = clearSynthSequenceTrigger.process(params[PARAM_SEQUENCER_2_BUTTON].getValue() > 0.5f);
        bool clearDrumSequenceRequested = clearDrumSequenceTrigger.process(params[PARAM_SEQUENCER_3_BUTTON].getValue() > 0.5f);
        bool enableAllActiveStepsRequested = enableAllActiveStepsTrigger.process(params[PARAM_SEQUENCER_4_BUTTON].getValue() > 0.5f);
        bool toggleLFOModeRequested = toggleLFOModeTrigger.process(params[PARAM_SEQUENCER_5_BUTTON].getValue() > 0.5f);
        bool toggleGateTimesLockRequested = gateTimesLockTrigger.process(params[PARAM_SEQUENCER_7_BUTTON].getValue() > 0.5f);
        bool toggleSyncHalfTempoRequested = syncHalfTempoTrigger.process(params[PARAM_SEQUENCER_8_BUTTON].getValue() > 0.5f);
        if (clearAllSequencesRequested) clearAllSequences();
        if (clearSynthSequenceRequested) clearSynthSequence();
        if (clearDrumSequenceRequested) clearDrumSequence();
        if (enableAllActiveStepsRequested) enableAllActiveSteps();
        if (toggleLFOModeRequested) lfoSampleAndHoldMode = !lfoSampleAndHoldMode;
        if (toggleGateTimesLockRequested) gateTimesLocked = !gateTimesLocked;
        if (toggleSyncHalfTempoRequested) syncHalfTempo = !syncHalfTempo;
    }
}

auto Clonotribe::processInputTriggers(float inputPitch, float gate, bool gateTimeHeld) -> std::tuple<float, float, bool, bool> {
    float finalInputPitch = inputPitch;
    float finalGate = gate;
    bool cvGateTriggered = gateTrigger.process(gate > 1.0f);
    bool combinedGateTriggered = cvGateTriggered;
    if (ribbon.touching) {
        if (gateTimeHeld) {
            finalInputPitch = inputPitch;
            finalGate = gate;
            combinedGateTriggered = cvGateTriggered;
        } else {
            finalInputPitch = ribbon.getCV();
            finalGate = ribbon.getGate();
            combinedGateTriggered = cvGateTriggered || (finalGate > 1.0f);
        }
    }
    bool gateTriggered = combinedGateTriggered;
    return {finalInputPitch, finalGate, gateTriggered, cvGateTriggered};
}
#include "../clonotribe.hpp"

void Clonotribe::process(const ProcessArgs& args) {
    // 1. Read all parameters and state
    auto [cutoff, lfoIntensity, lfoRate, noiseLevel, resonance, rhythmVolume, tempo, volume, octave, envelopeType, lfoMode, lfoTarget, lfoWaveform, ribbonMode, waveform] = readParameters();

    // 2. Update DSP/UI state
    updateDSPState(volume, rhythmVolume, lfoIntensity, ribbonMode, octave);

    // 3. Handle triggers and button logic
    handleMainTriggers();

    // 4. Handle drum part selection and sequencer tempo
    handleDrumSelectionAndTempo(tempo);

    // 5. Step and active step logic
    handleStepButtons();
    handleActiveStep();

    // 6. Gate time and special button logic
    bool gateTimeHeld = params[PARAM_GATE_TIME_BUTTON].getValue() > 0.5f;
    this->gateTimeHeld = gateTimeHeld;
    handleSpecialGateTimeButtons(gateTimeHeld);

    // 7. Drum rolls
    handleDrumRolls(args, gateTimeHeld);

    // 8. Input processing (CV, Gate, Ribbon)
    float cvVoltage = inputs[INPUT_CV_CONNECTOR].getVoltage();
    float inputPitch = cvVoltage + octave;
    float gate = inputs[INPUT_GATE_CONNECTOR].getVoltage();
    auto [finalInputPitch, finalGate, gateTriggered, cvGateTriggered] = processInputTriggers(inputPitch, gate, gateTimeHeld);

    // 9. Manual envelope/lfo triggering (when not playing)
    if (!sequencer.playing) {
        if (gateTriggered) {
            envelope.trigger();
            gateActive = true;
            if (lfoMode == 0) {
                lfo.trigger();
            }
        }
    }
    if (finalGate < 0.5f && gateActive) {
        envelope.gateOff();
        gateActive = false;
    }

    // 10. Sequencer, modulation, and output logic (unchanged, continues below...)
        float syncSignal = inputs[INPUT_SYNC_CONNECTOR].getVoltage();
        
        float effectiveSyncSignal = syncSignal;
        if (syncHalfTempo && sequencer.externalSync) {
            static dsp::SchmittTrigger syncHalfTrigger;
            bool syncTriggered = syncHalfTrigger.process(syncSignal > 1.0f);
            if (syncTriggered) {
                syncDivideCounter++;
                if (syncDivideCounter >= 2) {
                    syncDivideCounter = 0;
                    effectiveSyncSignal = 5.0f; // Send trigger to sequencer
                } else {
                    effectiveSyncSignal = 0.0f; // Skip this trigger
                }
            } else {
                effectiveSyncSignal = 0.0f;
            }
        }
        
        float ribbonGateTimeMod = 0.5f; // Default neutral value
        if (!gateTimesLocked) {
            // Only apply gate time modulation if not locked
            ribbonGateTimeMod = gateTimeHeld && ribbon.touching ? ribbon.getGateTimeMod() : 0.5f;
        }
        float ribbonVolumeAutomation = ribbon.getVolumeAutomation();

        auto seqOutput = sequencer.process(args.sampleTime, finalInputPitch, finalGate, effectiveSyncSignal, ribbonGateTimeMod);

        // Handle sequencer/drum state and step changes
        handleSequencerAndDrumState(seqOutput, finalInputPitch, finalGate, gateTriggered);
        
        float finalPitch = finalInputPitch; 
        float finalSequencerGate = finalGate;
        
        if (sequencer.playing) {
            // Gate Time Hold feature: CV input or ribbon controller can override pitch during sequencer playback
            if (ribbon.touching && !gateTimeHeld) {
                // Normal ribbon behavior: override pitch (but not when GATE TIME is held)
                finalPitch = ribbon.getCV(); // Use ribbon pitch
                // When ribbon is touched, still use sequencer gate timing (Gate Time Hold)
                finalSequencerGate = seqOutput.gate;
            } else if (gate > 1.0f) {
                finalPitch = inputPitch; // Use CV input pitch when gate is active
                // When CV gate is active, use the larger of CV gate or sequencer gate
                finalSequencerGate = std::max(gate, seqOutput.gate);
            } else {
                finalPitch = seqOutput.pitch; // Use sequencer pitch
                finalSequencerGate = seqOutput.gate; // Use sequencer gate timing
            }
        } else {
            // When sequencer is not playing, use the manual inputs (CV/Gate or ribbon)
            finalPitch = finalInputPitch;
            finalSequencerGate = finalGate;
        } 
        
        // Handle envelope triggering for sequencer playback
        bool shouldTriggerEnv = false;
        if (sequencer.playing) {
            // When sequencer is playing, trigger on new steps with active gates
            if (seqOutput.stepChanged && seqOutput.gate > 1.0f) {
                shouldTriggerEnv = true;
            }
            // Also respond to external gate input during sequencer playback
            if (cvGateTriggered) {
                shouldTriggerEnv = true;
            }
        }
        
        if (shouldTriggerEnv) {
            envelope.trigger();
            if (lfoMode == 0) {
                lfo.trigger();
            }
        }

        float actualLfoRate = 1.0f;
        bool isOneShot = false;
        
        bool isSampleAndHold = lfoSampleAndHoldMode && (lfoMode == 0);
        
        switch (lfoMode) {
            case 0: // 1-Shot (or Sample & Hold when enabled)
                actualLfoRate = rack::math::rescale(lfoRate, 0.0f, 1.0f, 1.0f, 5.0f);
                isOneShot = !isSampleAndHold; // Disable one-shot if in Sample & Hold mode
                break;
            case 1: // Slow
                actualLfoRate = rack::math::rescale(lfoRate, 0.0f, 1.0f, 0.05f, 18.0f);
                break;
            case 2: // Fast
                actualLfoRate = rack::math::rescale(lfoRate, 0.0f, 1.0f, 1.0f, 5000.0f);
                break;
            default:
                actualLfoRate = 1.0f;
        }
        lfo.setRate(actualLfoRate);
        lfo.setOneShot(isOneShot);
        lfo.setSampleAndHold(isSampleAndHold); // New method for Sample & Hold mode
        
        float lfoValue = lfo.process(args.sampleTime, static_cast<clonotribe::LFO::Waveform>(lfoWaveform));
        
        float pitchMod = 0.0f;
        float cutoffMod = 0.0f;
        
        switch (lfoTarget) {
            case 0: // VCF only
                cutoffMod = lfoValue * lfoIntensity * 0.5f;
                break;
            case 1: // VCO+VCF  
                pitchMod = lfoValue * lfoIntensity * 0.2f;
                cutoffMod = lfoValue * lfoIntensity * 0.5f;
                break;
            case 2: // VCO only
                pitchMod = lfoValue * lfoIntensity * 0.2f;
        }

        vco.setPitch(finalPitch + pitchMod);
        
        float vcoOutput = 0.0f;
        switch (waveform) {
            case 0: // Square
                vcoOutput = vco.processSquare(args.sampleTime);
                break;
            case 1: // Triangle  
                vcoOutput = vco.processTriangle(args.sampleTime);
                break;
            case 2: // Sawtooth
                vcoOutput = vco.processSaw(args.sampleTime);
        }
        
        // Add noise
        float noise = noiseGenerator.process() * noiseLevel;
        float mixedSignal = vcoOutput + noise;
        
        // Mix with audio input if connected
        float audioIn = inputs[INPUT_AUDIO_CONNECTOR].getVoltage();
        mixedSignal += audioIn * 1.5f; // Even higher gain for testing

        // Set filter parameters with modulation (using normalized 0-1 values)
        vcf.setCutoff(cutoff + cutoffMod);
        vcf.setResonance(resonance);
        
        // Process through filter
        float filteredSignal = vcf.process(mixedSignal);

        float envValue = processEnvelope(envelopeType, envelope, args.sampleTime, finalSequencerGate);

        float finalOutput = processOutput(
            filteredSignal, volume, envValue, ribbonVolumeAutomation,
            rhythmVolume, args.sampleTime, getKickDrum(), getSnareDrum(), getHiHat(), noiseGenerator
        );
        
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(std::clamp(finalOutput * 5.0f, -10.0f, 10.0f));
        outputs[OUTPUT_CV_CONNECTOR].setVoltage(finalPitch);
        outputs[OUTPUT_GATE_CONNECTOR].setVoltage(finalSequencerGate);
        
        if (inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
            outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(inputs[INPUT_SYNC_CONNECTOR].getVoltage());
        } else {
            bool syncOut = syncPulse.process(args.sampleTime);
            outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(syncOut ? 5.0f : 0.0f);
        }

        lights[LIGHT_PLAY].setBrightness(sequencer.playing ? 1.0f : 0.0f);
        lights[LIGHT_REC].setBrightness(sequencer.recording ? 1.0f : 0.0f);
        lights[LIGHT_FLUX].setBrightness(sequencer.fluxMode ? 1.0f : 0.0f);
        lights[LIGHT_SYNTH].setBrightness(selectedDrumPart == 0 ? 1.0f : 0.0f);
        lights[LIGHT_BASSDRUM].setBrightness(selectedDrumPart == 1 ? 1.0f : 0.0f);
        lights[LIGHT_SNARE].setBrightness(selectedDrumPart == 2 ? 1.0f : 0.0f);
        lights[LIGHT_HIGHHAT].setBrightness(selectedDrumPart == 3 ? 1.0f : 0.0f);
        
        updateStepLights(seqOutput);
}



void Clonotribe::handleActiveStep() {
    bool activeStepHeld = params[PARAM_ACTIVE_STEP_BUTTON].getValue() > 0.5f;

    if (activeStepHeld && !activeStepWasPressed) {
        activeStepWasPressed = true;
        activeStepActive = true;
        // Only toggle skipping (not muting) for synth part, and only on button press
        if (selectedDrumPart == 0 && selectedStepForEditing >= 0 && selectedStepForEditing < 8) {
            toggleActiveStep(selectedStepForEditing);
        }
    } else if (!activeStepHeld && activeStepWasPressed) {
        activeStepWasPressed = false;
        activeStepActive = false;
    }
}

void Clonotribe::handleDrumRolls(const ProcessArgs& args, bool gateTimeHeld) {
    static float rollTimer = 0.0f;
    
    if (gateTimeHeld && ribbon.touching && selectedDrumPart > 0) {
        float rollIntensity = ribbon.getDrumRollIntensity();
        float rollRate = rollIntensity * 50.0f + 1.0f;        
        rollTimer += args.sampleTime * rollRate;
        
        if (rollTimer >= 1.0f) {
            rollTimer -= 1.0f;
            switch (selectedDrumPart) {
                case 1: getKickDrum().reset(); break;
                case 2: getSnareDrum().reset(); break;
                case 3: getHiHat().reset(); break;
            }
        }
    } else {
        rollTimer = 0.0f;
    }
}


bool Clonotribe::isStepActiveInCurrentMode(int step) {
    if (activeStepActive) {
        if (selectedDrumPart == 0) {
            int idx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(step, false) : step;
            return (idx >= 0 && idx < sequencer.getStepCount()) && !sequencer.isStepSkipped(idx);
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    } else {
        if (selectedDrumPart == 0) {
            int idx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(step, false) : step;
            return (idx >= 0 && idx < sequencer.getStepCount()) && !sequencer.isStepMuted(idx);
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    }
}

void Clonotribe::clearAllSequences() {
    clearSynthSequence();
    clearDrumSequence();
}

void Clonotribe::clearSynthSequence() {
    int stepCount = sequencer.getStepCount();
    for (int i = 0; i < stepCount; i++) {
        sequencer.setStepSkipped(i, false);
        sequencer.setStepMuted(i, false);
        sequencer.steps[i].pitch = 0.0f;
        sequencer.steps[i].gate = 5.0f;
        sequencer.steps[i].gateTime = 0.8f;
    }
}


void Clonotribe::clearDrumSequence() {
    for (int d = 0; d < 3; d++) {
        for (int s = 0; s < 8; s++) {
            drumPatterns[d][s] = false;
        }
    }
}

void Clonotribe::enableAllActiveSteps() {
    if (selectedDrumPart == 0) {
        int stepCount = sequencer.getStepCount();
        for (int i = 0; i < stepCount; i++) {
            sequencer.setStepSkipped(i, false);
        }
    } else {
        int drumIndex = selectedDrumPart - 1;
        if (drumIndex >= 0 && drumIndex < 3) {
            for (int s = 0; s < 8; s++) {
                drumPatterns[drumIndex][s] = true;
            }
        }
    }
}