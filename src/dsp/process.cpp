#include "../clonotribe.hpp"
#include <tuple>

auto Clonotribe::readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, float, int, int, int, int, int, int> {
    auto getParamWithCV = [this](int paramId, int inputId) -> float {
        float value = params[paramId].getValue();
        if (inputs[inputId].isConnected()) {
            float cvVoltage = inputs[inputId].getVoltage();
            value = std::clamp((cvVoltage + 5.0f) * 0.1f, 0.0f, 1.0f);
            getParamQuantity(paramId)->setDisplayValue(value);
        }
        return value;
    };

    if (paramCache.needsUpdate()) {
        paramCache.cutoff = getParamWithCV(PARAM_VCF_CUTOFF_KNOB, INPUT_VCF_CUTOFF_CONNECTOR);
        paramCache.lfoIntensity = getParamWithCV(PARAM_LFO_INTERVAL_KNOB, INPUT_LFO_INTENSITY_CONNECTOR);
        
        if (!inputs[INPUT_LFO_RATE_CONNECTOR].isConnected()) {
            paramCache.lfoRate = params[PARAM_LFO_RATE_KNOB].getValue();
        } else {
            paramCache.lfoRate = params[PARAM_LFO_RATE_KNOB].getValue();
        }

        paramCache.noiseLevel = getParamWithCV(PARAM_NOISE_KNOB, INPUT_NOISE_CONNECTOR);
        paramCache.resonance = getParamWithCV(PARAM_VCF_PEAK_KNOB, INPUT_VCF_PEAK_CONNECTOR);
        paramCache.rhythmVolume = params[PARAM_RHYTHM_VOLUME_KNOB].getValue();
        paramCache.tempo = params[PARAM_SEQUENCER_TEMPO_KNOB].getValue();
        paramCache.volume = getParamWithCV(PARAM_VCA_LEVEL_KNOB, INPUT_VCA_CONNECTOR);
        paramCache.distortion = getParamWithCV(PARAM_DISTORTION_KNOB, INPUT_DISTORTION_CONNECTOR);
        
        float octaveSwitch = params[PARAM_VCO_OCTAVE_KNOB].getValue();
        if (inputs[INPUT_VCO_OCTAVE_CONNECTOR].isConnected()) {
            float cvVoltage = inputs[INPUT_VCO_OCTAVE_CONNECTOR].getVoltage();
            octaveSwitch = std::clamp((cvVoltage + 5.0f) * 0.5f, 0.0f, 5.0f);
            getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->setDisplayValue(octaveSwitch);
        }
        paramCache.octave = octaveSwitch - 3.0f;
        
        paramCache.envelopeType = (int) params[PARAM_ENVELOPE_FORM_SWITCH].getValue();
        paramCache.lfoMode = (int) params[PARAM_LFO_MODE_SWITCH].getValue();
        paramCache.lfoTarget = (int) params[PARAM_LFO_TARGET_SWITCH].getValue();
        paramCache.lfoWaveform = (int) params[PARAM_LFO_WAVEFORM_SWITCH].getValue();
        paramCache.ribbonMode = (int) params[PARAM_RIBBON_RANGE_SWITCH].getValue();
        paramCache.waveform = (int) params[PARAM_VCO_WAVEFORM_SWITCH].getValue();
        
        paramCache.resetUpdateCounter();
    }
    
    return {paramCache.cutoff, paramCache.lfoIntensity, paramCache.lfoRate, paramCache.noiseLevel, 
            paramCache.resonance, paramCache.rhythmVolume, paramCache.tempo, paramCache.volume, 
            paramCache.octave, paramCache.distortion, paramCache.envelopeType, paramCache.lfoMode, paramCache.lfoTarget, 
            paramCache.lfoWaveform, paramCache.ribbonMode, paramCache.waveform};
}

void Clonotribe::updateDSPState(float volume, float rhythmVolume, float lfoIntensity, int ribbonMode, float octave) {
    bool synthActive = true;
    bool filterActive = true;
    bool lfoActive = (lfoIntensity > 0.01f);
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
            envelope.gateOff();
            gateActive = false;
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
    auto [cutoff, lfoIntensity, lfoRate, noiseLevel, resonance, rhythmVolume, tempo, volume, octave, distortion, envelopeType, lfoMode, lfoTarget, lfoWaveform, ribbonMode, waveform] = readParameters();

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
        
        // Handle LFO clock input - when connected, sync LFO to clock regardless of mode
        bool shouldTriggerLfoFromClock = false;
        bool useClockInput = inputs[INPUT_LFO_RATE_CONNECTOR].isConnected();
        
        if (useClockInput) {
            float clockVoltage = inputs[INPUT_LFO_RATE_CONNECTOR].getVoltage();
            if (lfoClockTrigger.process(rack::math::rescale(clockVoltage, 0.1f, 2.f, 0.f, 1.f))) {
                shouldTriggerLfoFromClock = true;
            }
        }
        
        if (shouldTriggerLfoFromClock) {
            // Reset LFO phase on each clock pulse
            lfo.phase = 0.0f;
        }

        static float cachedLfoRate = -1.0f;
        static int cachedLfoMode = -1;
        static bool cachedSampleAndHold = false;
        static bool cachedClockInput = false;
        
        if (cachedLfoRate != lfoRate || cachedLfoMode != lfoMode || cachedSampleAndHold != lfoSampleAndHoldMode || cachedClockInput != useClockInput) {
            float actualLfoRate = 1.0f;
            bool isOneShot = false;
            bool isSampleAndHold = lfoSampleAndHoldMode && (lfoMode == 0);
            
            if (useClockInput) {
                // When using clock input, set LFO to free-running mode at 1Hz
                // The clock input will control the timing by resetting phase
                actualLfoRate = 1.0f;
                isOneShot = false;  // Always free-running when using clock
            } else {
                // Use knob control for rate when no clock input
                switch (lfoMode) {
                    case 0: // 1-Shot (or Sample & Hold when enabled)
                        actualLfoRate = 1.0f + lfoRate * 4.0f;
                        isOneShot = !isSampleAndHold;
                        break;
                    case 1: // Slow
                        actualLfoRate = 0.05f + lfoRate * 17.95f;
                        break;
                    case 2: // Fast
                        actualLfoRate = 1.0f + lfoRate * 4999.0f;
                        break;
                }
            }
            
            lfo.setRate(actualLfoRate);
            lfo.setOneShot(isOneShot);
            lfo.setSampleAndHold(isSampleAndHold);
            
            // Update cache
            cachedLfoRate = lfoRate;
            cachedLfoMode = lfoMode;
            cachedSampleAndHold = lfoSampleAndHoldMode;
            cachedClockInput = useClockInput;
        }
        
        float lfoValue = lfo.process(args.sampleTime, static_cast<clonotribe::LFO::Waveform>(lfoWaveform));
        
        float pitchMod = 0.0f;
        float cutoffMod = 0.0f;
        
        // Use bit flags for faster branching
        bool modulateVCF = (lfoTarget == 0 || lfoTarget == 1);
        bool modulateVCO = (lfoTarget == 1 || lfoTarget == 2);
        
        if (modulateVCF) cutoffMod = lfoValue * lfoIntensity * 0.5f;
        if (modulateVCO) pitchMod = lfoValue * lfoIntensity * 4.0f;

        vco.setPitch(finalPitch + pitchMod);
        
        static int cachedWaveform = -1;
        static float (VCO::*vcoProcessFunction)(float) = nullptr;
        
        if (cachedWaveform != waveform) {
            switch (waveform) {
                case 0: vcoProcessFunction = &VCO::processSquare; break;
                case 1: vcoProcessFunction = &VCO::processTriangle; break;
                case 2: vcoProcessFunction = &VCO::processSaw; break;
                default: vcoProcessFunction = &VCO::processSquare;
            }
            cachedWaveform = waveform;
        }
        
        float vcoOutput = (vco.*vcoProcessFunction)(args.sampleTime);
        
        // Add noise
        float noise = noiseGenerator.process() * noiseLevel;
        float mixedSignal = vcoOutput + noise;
        
        // Mix with audio input if connected
        float audioIn = inputs[INPUT_AUDIO_CONNECTOR].getVoltage();
        mixedSignal += audioIn * 1.5f;
        
        // Auto-gate: If audio input is present and strong enough, open the gate
        bool audioGateActive = false;
        if (inputs[INPUT_AUDIO_CONNECTOR].isConnected() && std::abs(audioIn) > 0.1f) {
            audioGateActive = true;
            // Trigger envelope if we have strong audio input signal or envelope is not active
            if (envelope.stage == clonotribe::Envelope::Stage::Off || std::abs(audioIn) > 2.0f) {
                envelope.trigger();
            }
        }

        float filteredSignal = filterProcessor.process(mixedSignal, cutoff + cutoffMod, resonance);

        // Use audio-triggered gate if active, otherwise use sequencer gate
        float effectiveGate = audioGateActive ? 5.0f : finalSequencerGate;
        float envValue = processEnvelope(envelopeType, envelope, args.sampleTime, effectiveGate);

        float finalOutput = processOutput(
            filteredSignal, volume, envValue, ribbonVolumeAutomation,
            rhythmVolume, args.sampleTime, noiseGenerator, seqOutput.step, distortion
        );
        
        // Gentle output limiting to prevent clipping and distortion
        finalOutput = clonotribe::FastMath::fastTanh(finalOutput * 0.8f) * 1.2f;
        
        // Improved output scaling for better levels
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(std::clamp(finalOutput * 4.0f, -10.0f, 10.0f));
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
                case 1: triggerKick(); break;
                case 2: triggerSnare(); break;
                case 3: triggerHihat(); break;
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