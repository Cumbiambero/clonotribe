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
        float bpm = rescale(tempo, 0.0f, 1.0f, minTempo, maxTempo);
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
                actualLfoRate = rescale(lfoRate, 0.0f, 1.0f, 1.0f, 5.0f);
                isOneShot = !isSampleAndHold; // Disable one-shot if in Sample & Hold mode
                break;
            case 1: // Slow
                actualLfoRate = rescale(lfoRate, 0.0f, 1.0f, 0.05f, 18.0f);
                break;
            case 2: // Fast
                actualLfoRate = rescale(lfoRate, 0.0f, 1.0f, 1.0f, 5000.0f);
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

        // Set filter parameters with modulation
        float cutoffFreq = rescale(cutoff + cutoffMod, 0.0f, 1.0f, 80.0f, 8000.0f);
        float res = rescale(resonance, 0.0f, 1.0f, 0.0f, 3.5f);
        vcf.setCutoff(cutoffFreq);
        vcf.setResonance(res);
        
        // Process through filter
        float filteredSignal = vcf.process(mixedSignal, args.sampleRate, noiseGenerator);

        float envValue = processEnvelope(envelopeType, envelope, args.sampleTime, finalSequencerGate);

        float finalOutput = processOutput(
            filteredSignal, volume, envValue, ribbonVolumeAutomation,
            rhythmVolume, args.sampleTime, kickDrum, snareDrum, hiHat, noiseGenerator
        );
        
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(clamp(finalOutput * 5.0f, -10.0f, 10.0f));
        outputs[OUTPUT_CV_CONNECTOR].setVoltage(finalPitch);
        outputs[OUTPUT_GATE_CONNECTOR].setVoltage(finalSequencerGate);
        
        if (inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
            // Pass through input sync
            outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(inputs[INPUT_SYNC_CONNECTOR].getVoltage());
        } else {
            // Generate sync output pulses when playing
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


void Clonotribe::handleStepButtons() {
    for (int i = 0; i < 8; i++) {
        bool stepPressed = stepTriggers[i].process(params[PARAM_SEQUENCER_1_BUTTON + i].getValue() > 0.5f);
        if (stepPressed) {
            selectedStepForEditing = i;
            
            if (sequencer.isInSixteenStepMode() && selectedDrumPart == 0) {
                // In 16-step mode for synth part, check if we're editing main step or sub-step
                if (gateTimeHeld) {
                    // GATE_TIME + step button = toggle sub-step (odd indices)
                    int subStepIndex = sequencer.getStepIndex(i, true); // Get sub-step index
                    bool currentState = sequencer.isStepActive(subStepIndex);
                    sequencer.setStepActive(subStepIndex, !currentState);
                } else {
                    // Normal step button = toggle main step (even indices)
                    int mainStepIndex = sequencer.getStepIndex(i, false); // Get main step index
                    bool currentState = sequencer.isStepActive(mainStepIndex);
                    sequencer.setStepActive(mainStepIndex, !currentState);
                }
            } else {
                // 8-step mode or drum parts - use original behavior
                toggleStepInCurrentMode(i);
            }
        }
    }
}

void Clonotribe::handleActiveStep() {
    bool activeStepHeld = params[PARAM_ACTIVE_STEP_BUTTON].getValue() > 0.5f;
    
    if (activeStepHeld && !activeStepWasPressed) {
        activeStepWasPressed = true;
        activeStepActive = true;
        
        // Copy current state - expand for 16-step mode
        for (int i = 0; i < 16; i++) {
            if (i < sequencer.getStepCount()) {
                activeStepsSequencerSteps[i] = sequencer.isStepActive(i);
            } else {
                activeStepsSequencerSteps[i] = false;
            }
        }
        for (int d = 0; d < 3; d++) {
            for (int i = 0; i < 8; i++) {
                activeStepsDrumPatterns[d][i] = drumPatterns[d][i];
            }
        }
        
        // Toggle the selected step
        if (selectedStepForEditing >= 0 && selectedStepForEditing < 8) {
            if (selectedDrumPart == 0) {
                // For synth part, toggle appropriate step based on 16-step mode
                if (sequencer.isInSixteenStepMode()) {
                    // In 16-step mode, toggle main step (even index)
                    int stepIndex = sequencer.getStepIndex(selectedStepForEditing, false);
                    if (stepIndex < 16) {
                        activeStepsSequencerSteps[stepIndex] = !activeStepsSequencerSteps[stepIndex];
                    }
                } else {
                    // In 8-step mode, toggle normally
                    activeStepsSequencerSteps[selectedStepForEditing] = !activeStepsSequencerSteps[selectedStepForEditing];
                }
            } else {
                int drumIndex = selectedDrumPart - 1;
                if (drumIndex >= 0 && drumIndex < 3) {
                    activeStepsDrumPatterns[drumIndex][selectedStepForEditing] = !activeStepsDrumPatterns[drumIndex][selectedStepForEditing];
                }
            }
        }
    } else if (!activeStepHeld && activeStepWasPressed) {
        // ACTIVE STEP released - return to original sequence
        activeStepWasPressed = false;
        activeStepActive = false;
    }
}

void Clonotribe::handleDrumRolls(const ProcessArgs& args, bool gateTimeHeld) {
    static float rollTimer = 0.0f;
    
    if (gateTimeHeld && ribbon.touching && selectedDrumPart > 0) {
        float rollIntensity = ribbon.getDrumRollIntensity();
        float rollRate = rollIntensity * 50.0f + 1.0f; // 1-51 Hz roll rate
        rollTimer += args.sampleTime * rollRate;
        
        if (rollTimer >= 1.0f) {
            rollTimer -= 1.0f;
            switch (selectedDrumPart) {
                case 1: kickDrum.trigger(); break;
                case 2: snareDrum.trigger(); break;
                case 3: hiHat.trigger(); break;
            }
        }
    } else {
        rollTimer = 0.0f;
    }
}

void Clonotribe::updateStepLights(const clonotribe::Sequencer::SequencerOutput& seqOutput) {
    static float blinkTimer = 0.0f;
    blinkTimer += 1.0f / APP->engine->getSampleRate(); // Increment by sample time
    bool blinkState = fmodf(blinkTimer, 0.5f) < 0.25f; // Blink at 2 Hz
    
    for (int i = 0; i < 8; i++) {
        float brightness = 0.0f;
        
        if (sequencer.isInSixteenStepMode() && selectedDrumPart == 0) {
            // 16-step mode: LEDs represent both main and sub-steps
            int mainStepIndex = sequencer.getStepIndex(i, false); // Even index (0,2,4,6,8,10,12,14)
            int subStepIndex = sequencer.getStepIndex(i, true);   // Odd index (1,3,5,7,9,11,13,15)
            
            bool mainStepActive = sequencer.isStepActive(mainStepIndex);
            bool subStepActive = sequencer.isStepActive(subStepIndex);
            
            // Current playing step gets special treatment
            if (sequencer.playing && (seqOutput.step == mainStepIndex || seqOutput.step == subStepIndex)) {
                if (seqOutput.step == mainStepIndex) {
                    brightness = 1.0f; // Solid bright for main step
                } else {
                    brightness = blinkState ? 1.0f : 0.5f; // Blinking for sub-step
                }
            } else {
                // Not currently playing - show step state
                if (mainStepActive && subStepActive) {
                    brightness = blinkState ? 0.6f : 0.3f; // Blink between bright and dim
                } else if (mainStepActive) {
                    brightness = 0.3f; // Solid dim for main step only
                } else if (subStepActive) {
                    brightness = blinkState ? 0.3f : 0.0f; // Blink dim for sub-step only
                } else {
                    brightness = 0.0f; // Off if neither active
                }
                
                // Recording step indicator
                if (sequencer.recording && !sequencer.playing && selectedDrumPart == 0) {
                    if (sequencer.recordingStep == mainStepIndex || sequencer.recordingStep == subStepIndex) {
                        brightness = 0.6f;
                    }
                }
                
                // Active steps mode highlighting
                if (activeStepActive) {
                    if (i == selectedStepForEditing) {
                        brightness = (mainStepActive || subStepActive) ? 1.0f : 0.2f;
                    } else {
                        if (mainStepActive || subStepActive) {
                            brightness = 0.4f;
                        } else {
                            brightness = 0.05f;
                        }
                    }
                }
            }
        } else {
            // 8-step mode or drum parts - original behavior
            // Current playing step gets full brightness
            if (sequencer.playing && seqOutput.step == i) {
                brightness = 1.0f;
            } else {
                bool stepActive = isStepActiveInCurrentMode(i);
                brightness = stepActive ? 0.3f : 0.0f;
                
                // Recording step indicator
                if (sequencer.recording && !sequencer.playing && selectedDrumPart == 0 && i == sequencer.recordingStep) {
                    brightness = 0.6f;
                }
                
                // Active steps mode highlighting
                if (activeStepActive) {
                    if (i == selectedStepForEditing) {
                        brightness = stepActive ? 1.0f : 0.2f;
                    } else {
                        brightness = stepActive ? 0.4f : 0.05f;
                    }
                }
            }
        }
        
        lights[LIGHT_SEQUENCER_1 + i].setBrightness(brightness);
    }
}

bool Clonotribe::isStepActiveInCurrentMode(int step) {
    if (activeStepActive) {
        if (selectedDrumPart == 0) {
            if (sequencer.isInSixteenStepMode()) {
                int stepIndex = sequencer.getStepIndex(step, false);
                return (stepIndex < 16) ? activeStepsSequencerSteps[stepIndex] : false;
            } else {
                return activeStepsSequencerSteps[step];
            }
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? activeStepsDrumPatterns[drumIndex][step] : false;
        }
    } else {
        if (selectedDrumPart == 0) {
            if (sequencer.isInSixteenStepMode()) {
                int stepIndex = sequencer.getStepIndex(step, false);
                return sequencer.isStepActive(stepIndex);
            } else {
                return sequencer.isStepActive(step);
            }
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    }
}

void Clonotribe::toggleStepInCurrentMode(int step) {
    if (activeStepActive) {
        if (selectedDrumPart == 0) {
            if (sequencer.isInSixteenStepMode()) {
                int stepIndex = sequencer.getStepIndex(step, false);
                if (stepIndex < 16) {
                    activeStepsSequencerSteps[stepIndex] = !activeStepsSequencerSteps[stepIndex];
                }
            } else {
                activeStepsSequencerSteps[step] = !activeStepsSequencerSteps[step];
            }
        } else {
            int drumIndex = selectedDrumPart - 1;
            if (drumIndex >= 0 && drumIndex < 3) {
                activeStepsDrumPatterns[drumIndex][step] = !activeStepsDrumPatterns[drumIndex][step];
            }
        }
    } else {
        if (selectedDrumPart == 0) {
            if (sequencer.isInSixteenStepMode()) {
                int stepIndex = sequencer.getStepIndex(step, false);
                sequencer.setStepActive(stepIndex, !sequencer.isStepActive(stepIndex));
            } else {
                sequencer.setStepActive(step, !sequencer.isStepActive(step));
            }
        } else {
            int drumIndex = selectedDrumPart - 1;
            if (drumIndex >= 0 && drumIndex < 3) {
                drumPatterns[drumIndex][step] = !drumPatterns[drumIndex][step];
            }
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
        sequencer.setStepActive(i, false);
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
            sequencer.setStepActive(i, true);
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