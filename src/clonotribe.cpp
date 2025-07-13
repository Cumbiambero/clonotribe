#include "clonotribe.hpp"
#include "ui/transparentbutton.hpp"
#include "ui/ribbonwidget.hpp"

Clonotribe::Clonotribe() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        
        for (int i = 0; i < 8; i++) {
            sequencer.steps[i].active = true; // All steps enabled by default
            sequencer.steps[i].pitch = 0.f;
            sequencer.steps[i].gate = 5.f;
            sequencer.steps[i].gateTime = 0.8f;
        }
        
        configSwitch(PARAM_VCO_WAVEFORM_SWITCH, 0.f, 2.f, 0.f, "VCO Waveform", {"Square", "Triangle", "Sawtooth"});
        configSwitch(PARAM_RIBBON_RANGE_SWITCH, 0.f, 2.f, 0.f, "Ribbon Range", {"Key", "Narrow", "Wide"});
        configSwitch(PARAM_ENVELOPE_FORM_SWITCH, 0.f, 2.f, 0.f, "Envelope",  {"Attack", "Gate", "Decay"});
        configSwitch(PARAM_LFO_TARGET_SWITCH, 0.f, 2.f, 0.f, "LFO Target",  {"VCF", "VCO+VCF", "VCO"});
        configSwitch(PARAM_LFO_MODE_SWITCH, 0.f, 2.f, 0.f, "LFO Mode", {"1 Shot", "Slow", "Fast"});
        configSwitch(PARAM_LFO_WAVEFORM_SWITCH, 0.f, 2.f, 0.f, "LFO Waveform",  {"Square", "Triangle", "Sawtooth"});
        
        configParam(PARAM_VCO_OCTAVE_KNOB, 0.f, 5.f, 2.f, "VCO Octave");
        getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->snapEnabled = true;

        configParam(PARAM_NOISE_KNOB, 0.f, 1.f, 0.f, "Noise Level", "%", 0.f, 100.f);
        
        configParam(PARAM_VCF_CUTOFF_KNOB, 0.f, 1.f, 0.7f, "VCF Cutoff");
        configParam(PARAM_VCF_PEAK_KNOB, 0.f, 1.f, 0.f, "VCF Peak (Resonance)");
        
        configParam(PARAM_VCA_LEVEL_KNOB, 0.f, 1.f, 0.8f, "VCA Level");
        
        configParam(PARAM_LFO_RATE_KNOB, 0.f, 1.f, 0.3f, "LFO Rate");
        configParam(PARAM_LFO_INTERVAL_KNOB, 0.f, 1.f, 0.f, "LFO Intensity");
        
        configParam(PARAM_RHYTM_VOLUME_KNOB, 0.f, 1.f, 0.f, "Rhythm Volume");
        configParam(PARAM_SEQUENCER_TEMPO_KNOB, 0.f, 1.f, 0.5f, "Sequencer Tempo", " BPM", 0.f, 120.f, 60.f);
        
        configButton(PARAM_SNARE_BUTTON, "Snare");
        configButton(PARAM_FLUX_BUTTON, "Flux");
        configButton(PARAM_REC_BUTTON, "Record");
        configButton(PARAM_SYNTH_BUTTON, "Synth");
        configButton(PARAM_BASSDRUM_BUTTON, "Bass Drum");
        configButton(PARAM_HIGHHAT_BUTTON, "Hi-Hat");
        configButton(PARAM_ACTIVE_STEP_BUTTON, "Active Step (F7)");
        configButton(PARAM_SEQUENCER_1_BUTTON, "Sequencer 1");
        configButton(PARAM_SEQUENCER_2_BUTTON, "Sequencer 2");
        configButton(PARAM_SEQUENCER_3_BUTTON, "Sequencer 3");
        configButton(PARAM_SEQUENCER_4_BUTTON, "Sequencer 4");
        configButton(PARAM_SEQUENCER_5_BUTTON, "Sequencer 5");
        configButton(PARAM_SEQUENCER_6_BUTTON, "Sequencer 6");
        configButton(PARAM_SEQUENCER_7_BUTTON, "Sequencer 7");
        configButton(PARAM_SEQUENCER_8_BUTTON, "Sequencer 8");
        configButton(PARAM_PLAY_BUTTON, "Play");
        configButton(PARAM_GATE_TIME_BUTTON, "Gate Time (F8)");
        
        configInput(INPUT_CV_CONNECTOR, "CV");
        configInput(INPUT_GATE_CONNECTOR, "Gate");
        configInput(INPUT_AUDIO_CONNECTOR, "Audio");
        configInput(INPUT_SYNC_CONNECTOR, "Sync");
        
        configOutput(OUTPUT_CV_CONNECTOR, "CV");
        configOutput(OUTPUT_GATE_CONNECTOR, "Gate");
        configOutput(OUTPUT_AUDIO_CONNECTOR, "Audio");
        configOutput(OUTPUT_SYNC_CONNECTOR, "Sync");
}

void Clonotribe::process(const ProcessArgs& args) {
        int octaveSwitch = (int)params[PARAM_VCO_OCTAVE_KNOB].getValue();
        float octave = octaveSwitch - 3.f;
        int waveform = (int)params[PARAM_VCO_WAVEFORM_SWITCH].getValue();
        float noiseLevel = params[PARAM_NOISE_KNOB].getValue();
        float cutoff = params[PARAM_VCF_CUTOFF_KNOB].getValue();
        float resonance = params[PARAM_VCF_PEAK_KNOB].getValue();
        float level = params[PARAM_VCA_LEVEL_KNOB].getValue();
        float lfoRate = params[PARAM_LFO_RATE_KNOB].getValue();
        float lfoIntensity = params[PARAM_LFO_INTERVAL_KNOB].getValue();
        int lfoTarget = (int)params[PARAM_LFO_TARGET_SWITCH].getValue();
        int lfoMode = (int)params[PARAM_LFO_MODE_SWITCH].getValue();
        int lfoWaveform = (int)params[PARAM_LFO_WAVEFORM_SWITCH].getValue();
        int envelopeType = (int)params[PARAM_ENVELOPE_FORM_SWITCH].getValue();
        float tempo = params[PARAM_SEQUENCER_TEMPO_KNOB].getValue();
        int ribbonMode = (int)params[PARAM_RIBBON_RANGE_SWITCH].getValue();
        float rhythmVolume = params[PARAM_RHYTM_VOLUME_KNOB].getValue();

        // Conditional processing optimization: only process active components
        bool synthActive = (selectedDrumPart == 0) || (level > 0.01f && rhythmVolume < 0.99f);
        bool lfoActive = synthActive && (lfoIntensity > 0.01f);
        bool filterActive = synthActive;
        
        // Set component active states
        lfo.active = lfoActive;
        vco.active = synthActive;
        vcf.setActive(filterActive);

        ribbon.setMode(ribbonMode);
        ribbon.setOctave(octave);
        
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
        
        bool synthPressed = drumTriggers[0].process(params[PARAM_SYNTH_BUTTON].getValue() > 0.5f);
        bool kickPressed = drumTriggers[1].process(params[PARAM_BASSDRUM_BUTTON].getValue() > 0.5f);
        bool snarePressed = drumTriggers[2].process(params[PARAM_SNARE_BUTTON].getValue() > 0.5f);
        bool hihatPressed = drumTriggers[3].process(params[PARAM_HIGHHAT_BUTTON].getValue() > 0.5f);
        
        if (synthPressed) selectedDrumPart = 0;
        if (kickPressed) selectedDrumPart = 1;
        if (snarePressed) selectedDrumPart = 2;
        if (hihatPressed) selectedDrumPart = 3;
        
        // Set sequencer tempo - disabled if SYNC IN is connected
        if (!inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
            float bpm = rescale(tempo, 0.f, 1.f, 60.f, 180.f); 
            sequencer.setTempo(bpm);
            sequencer.setExternalSync(false);
        } else {
            sequencer.setExternalSync(true);
        }
        
        handleStepButtons();
        handleActiveStep();
        
        // GATE_TIME acts as momentary button for gate time modulation and drum rolls
        bool gateTimeHeld = params[PARAM_GATE_TIME_BUTTON].getValue() > 0.5f;
        handleDrumRolls(args, gateTimeHeld);
        
        float cvVoltage = inputs[INPUT_CV_CONNECTOR].getVoltage();
        float inputPitch = cvVoltage + octave; // CV is already in 1V/Oct
        
        float gate = inputs[INPUT_GATE_CONNECTOR].getVoltage();
        
        float finalInputPitch = inputPitch;
        float finalGate = gate;
        
        // Separate gate triggers for recording vs playing
        bool cvGateTriggered = gateTrigger.process(gate > 1.f); // Pure CV/Gate input
        bool combinedGateTriggered = cvGateTriggered;
        
        // Ribbon controller behavior depends on GATE TIME button state
        if (ribbon.touching) {
            if (gateTimeHeld) {
                // When GATE TIME is held, ribbon does NOT emit notes
                // It only provides modulation for gate time (synth) or drum roll intensity (drums)
                // Keep using CV/Gate input for notes
                finalInputPitch = inputPitch;
                finalGate = gate;
                combinedGateTriggered = cvGateTriggered; // Only CV/Gate triggers notes
            } else {
                // Normal ribbon behavior - emit notes
                finalInputPitch = ribbon.getCV();
                finalGate = ribbon.getGate();
                // For playing, use combined gate (CV/Gate OR ribbon)
                combinedGateTriggered = cvGateTriggered || (finalGate > 1.f);
            }
        }
        
        bool gateTriggered = combinedGateTriggered;
        
        // Handle envelope triggering - avoid double triggers
        if (!sequencer.playing) {
            // When sequencer is not playing, respond to manual input triggers
            if (gateTriggered) {
                envelope.trigger();
                gateActive = true;
                if (lfoMode == 0) {
                    lfo.trigger();
                }
            }
        }
        // When sequencer is playing, envelope triggering is handled later based on sequencer steps
        
        if (finalGate < 0.5f && gateActive) {
            envelope.gateOff();
            gateActive = false;
        }

        float syncSignal = inputs[INPUT_SYNC_CONNECTOR].getVoltage();
        
        // Get ribbon controller modulations (firmware 2.1 features)
        float ribbonGateTimeMod = gateTimeHeld && ribbon.touching ? ribbon.getGateTimeMod() : 0.5f; // Apply gate time mod when GATE TIME is held and ribbon is touched
        float ribbonVolumeAutomation = ribbon.getVolumeAutomation();
        
        auto seqOutput = sequencer.process(args.sampleTime, finalInputPitch, finalGate, syncSignal, ribbonGateTimeMod);

        // Override sequencer output with active steps when ACTIVE STEP is active
        if (activeStepActive && sequencer.playing) {
            // Check if current step should be active in active steps sequence
            int currentStep = seqOutput.step;
            if (currentStep >= 0 && currentStep < 8) {
                if (selectedDrumPart == 0) {
                    if (!activeStepsSequencerSteps[currentStep]) {
                        seqOutput.gate = 0.f;
                    }
                } 
            }
        }
        
        if (sequencer.recording && selectedDrumPart == 0) {
            if (sequencer.fluxMode) {
                if (finalGate > 1.f) {
                    sequencer.recordFlux(finalInputPitch);
                }
            } else {
                if (sequencer.playing) {
                    // Record on any gate trigger (CV/Gate input OR ribbon touch)
                    if (gateTriggered) {
                        sequencer.recordNote(finalInputPitch, finalGate > 1.f ? finalGate : 5.f, 0.8f);
                    }
                } else {
                    // When not playing, record on gate trigger from any source
                    if (gateTriggered) {
                        sequencer.recordNoteToStep(sequencer.recordingStep, finalInputPitch, finalGate > 1.f ? finalGate : 5.f, 0.8f);
                        sequencer.recordingStep = (sequencer.recordingStep + 1) % 8;
                    }
                }
            }
        }
        
        // Trigger drums on step changes and generate sync pulses
        if (sequencer.playing && seqOutput.stepChanged) {
            int currentStep = seqOutput.step;
            if (activeStepActive) {
                if (activeStepsDrumPatterns[0][currentStep]) kickDrum.trigger();  // Kick
                if (activeStepsDrumPatterns[1][currentStep]) snareDrum.trigger(); // Snare
                if (activeStepsDrumPatterns[2][currentStep]) hiHat.trigger();     // Hi-hat
            } else {
                if (drumPatterns[0][currentStep]) kickDrum.trigger();  // Kick
                if (drumPatterns[1][currentStep]) snareDrum.trigger(); // Snare
                if (drumPatterns[2][currentStep]) hiHat.trigger();     // Hi-hat
            }
            
            // Generate sync pulse on step change
            syncPulse.trigger(1e-3f); // 1ms pulse
        }
        
        float finalPitch = finalInputPitch; 
        float finalSequencerGate = finalGate;
        
        if (sequencer.playing) {
            // Gate Time Hold feature: CV input or ribbon controller can override pitch during sequencer playback
            if (ribbon.touching && !gateTimeHeld) {
                // Normal ribbon behavior: override pitch (but not when GATE TIME is held)
                finalPitch = ribbon.getCV(); // Use ribbon pitch
                // When ribbon is touched, still use sequencer gate timing (Gate Time Hold)
                finalSequencerGate = seqOutput.gate;
            } else if (gate > 1.f) {
                finalPitch = inputPitch; // Use CV input pitch when gate is active
                // When CV gate is active, use the larger of CV gate or sequencer gate
                finalSequencerGate = std::max(gate, seqOutput.gate);
            } else {
                finalPitch = seqOutput.pitch; // Use sequencer pitch
                finalSequencerGate = seqOutput.gate; // Use sequencer gate timing
                
                // ACTIVE STEP works by temporarily changing step patterns, no additional muting needed here
            }
        } 
        
        // Handle envelope triggering for sequencer playback
        bool shouldTriggerEnv = false;
        if (sequencer.playing) {
            // When sequencer is playing, trigger on new steps with active gates
            if (seqOutput.stepChanged && seqOutput.gate > 1.f) {
                shouldTriggerEnv = true;
            }
        }
        // Note: Manual input triggering is handled above when sequencer is not playing
        
        if (shouldTriggerEnv) {
            envelope.trigger();
            if (lfoMode == 0) {
                lfo.trigger();
            }
        }

        float actualLfoRate = 1.f;
        bool isOneShot = false;
        switch (lfoMode) {
            case 0: // 1-Shot
                actualLfoRate = rescale(lfoRate, 0.f, 1.f, 1.f, 5.f);
                isOneShot = true;
                break;
            case 1: // Slow
                actualLfoRate = rescale(lfoRate, 0.f, 1.f, 0.05f, 18.f);
                break;
            case 2: // Fast
                actualLfoRate = rescale(lfoRate, 0.f, 1.f, 1.f, 5000.f);
                break;
            default:
                actualLfoRate = 1.f;
                break;
        }
        lfo.setRate(actualLfoRate);
        lfo.setOneShot(isOneShot);
        
        float lfoValue = lfo.process(args.sampleTime, lfoWaveform);
        
        float pitchMod = 0.f;
        float cutoffMod = 0.f;
        
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
                break;
        }

        vco.setPitch(finalPitch + pitchMod);
        
        float vcoOutput = 0.f;
        switch (waveform) {
            case 0: // Square
                vcoOutput = vco.processSquare(args.sampleTime);
                break;
            case 1: // Triangle  
                vcoOutput = vco.processTriangle(args.sampleTime);
                break;
            case 2: // Sawtooth
                vcoOutput = vco.processSaw(args.sampleTime);
                break;
        }
        
        // Add noise
        float noise = noiseDist(noiseEngine) * noiseLevel;
        float mixedSignal = vcoOutput + noise;
        
        // Mix with audio input if connected
        float audioIn = inputs[INPUT_AUDIO_CONNECTOR].getVoltage();
        mixedSignal += audioIn * 1.5f; // Even higher gain for testing

        // Set filter parameters with modulation
        float cutoffFreq = rescale(cutoff + cutoffMod, 0.f, 1.f, 80.f, 8000.f);
        float res = rescale(resonance, 0.f, 1.f, 0.f, 3.5f);
        vcf.setCutoff(cutoffFreq);
        vcf.setResonance(res);
        
        // Process through filter
        float filteredSignal = vcf.process(mixedSignal, args.sampleRate);
        
        // Process envelope based on type
        float envValue = 1.f;
        bool useGate = (finalSequencerGate > 1.f); // Use the final gate value
        
        switch (envelopeType) {
            case 0: // Attack
                envelope.setAttack(0.1f);
                envelope.setDecay(0.1f);
                envelope.setSustain(1.f);
                envelope.setRelease(0.1f);
                envValue = envelope.process(args.sampleTime);
                break;
            case 1: // Gate
                envValue = useGate ? 1.f : 0.f;
                break;
            case 2: // Decay
                envelope.setAttack(0.001f);
                envelope.setDecay(0.5f);
                envelope.setSustain(0.f);
                envelope.setRelease(0.001f);
                envValue = envelope.process(args.sampleTime);
                break;
        }
        
        // Apply VCA (level and envelope) with ribbon controller volume automation
        float volumeModulation = 1.0f + (ribbonVolumeAutomation * 0.5f); // ±50% volume change
        volumeModulation = clamp(volumeModulation, 0.1f, 2.0f);
        float finalOutput = filteredSignal * level * envValue * volumeModulation;
        
        // Process drums and mix with synth output
        float drumMix = 0.f;
        
        if (rhythmVolume > 0.f) {
            float kickOut = kickDrum.process(args.sampleTime);
            float snareOut = snareDrum.process(args.sampleTime, noiseGen);
            float hihatOut = hiHat.process(args.sampleTime, noiseGen);
            
            drumMix = (kickOut + snareOut + hihatOut) * rhythmVolume;
        }
        
        // Mix synth and drums
        finalOutput += drumMix;
        
        // Output the audio
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(clamp(finalOutput * 5.f, -10.f, 10.f));
        
        // Output CV and gate 
        outputs[OUTPUT_CV_CONNECTOR].setVoltage(finalPitch);
        outputs[OUTPUT_GATE_CONNECTOR].setVoltage(finalSequencerGate);
        
        // Pass sync through or generate sync output
        if (inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
            // Pass through input sync
            outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(inputs[INPUT_SYNC_CONNECTOR].getVoltage());
        } else {
            // Generate sync output pulses when playing
            bool syncOut = syncPulse.process(args.sampleTime);
            outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(syncOut ? 5.f : 0.f);
        }

        // Update light states
        lights[LIGHT_PLAY].setBrightness(sequencer.playing ? 1.f : 0.f);
        lights[LIGHT_REC].setBrightness(sequencer.recording ? 1.f : 0.f);
        lights[LIGHT_FLUX].setBrightness(sequencer.fluxMode ? 1.f : 0.f);
        
        // Part selection lights (exclusive)
        lights[LIGHT_SYNTH].setBrightness(selectedDrumPart == 0 ? 1.f : 0.f);
        lights[LIGHT_BASSDRUM].setBrightness(selectedDrumPart == 1 ? 1.f : 0.f);
        lights[LIGHT_SNARE].setBrightness(selectedDrumPart == 2 ? 1.f : 0.f);
        lights[LIGHT_HIGHHAT].setBrightness(selectedDrumPart == 3 ? 1.f : 0.f);
        
        updateStepLights(seqOutput);
}


// Simple transparent momentary button that integrates properly with VCV Rack


struct ClonotribeWidget : ModuleWidget {
    TransparentMomentaryButton* createTransparentButton(Vec pos, Vec size, Module* module, int paramId) {
        auto* button = createParam<TransparentMomentaryButton>(pos, module, paramId);
        button->box.size = size;
        return button;
    }
    
    ClonotribeWidget(Clonotribe* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));

        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0, 42.0)), module, Clonotribe::PARAM_VCO_WAVEFORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0, 62.5)), module, Clonotribe::PARAM_RIBBON_RANGE_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(77.0, 62.5)), module, Clonotribe::PARAM_ENVELOPE_FORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(94.0, 62.5)), module, Clonotribe::PARAM_LFO_TARGET_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(109.0, 62.5)), module, Clonotribe::PARAM_LFO_MODE_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(123.0, 62.5)), module, Clonotribe::PARAM_LFO_WAVEFORM_SWITCH));

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 42.0)), module, Clonotribe::PARAM_VCO_OCTAVE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5, 42.0)), module, Clonotribe::PARAM_VCF_CUTOFF_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.0, 42.0)), module, Clonotribe::PARAM_VCA_LEVEL_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(98.0, 42.0)), module, Clonotribe::PARAM_LFO_RATE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(119.0, 42.0)), module, Clonotribe::PARAM_LFO_INTERVAL_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0, 42.0)), module, Clonotribe::PARAM_RHYTM_VOLUME_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 62.5)), module, Clonotribe::PARAM_NOISE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5, 62.5)), module, Clonotribe::PARAM_VCF_PEAK_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0, 62.5)), module, Clonotribe::PARAM_SEQUENCER_TEMPO_KNOB));

        addParam(createTransparentButton(mm2px(Vec(30.25, 103.25)), Vec(51.0, 18.0), module, Clonotribe::PARAM_PLAY_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(11.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SYNTH_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(20.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_BASSDRUM_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(30.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SNARE_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(39.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_HIGHHAT_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(11.0, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_FLUX_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(20.5, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_REC_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(68.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_1_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(78.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_2_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(88.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_3_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(97.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_4_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(107.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_5_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(117.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_6_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(126.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_7_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(136.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_8_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(49.25, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_ACTIVE_STEP_BUTTON));
        addParam(createTransparentButton(mm2px(Vec(49.25, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_GATE_TIME_BUTTON));
       
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(6.0, 8.6)), module, Clonotribe::INPUT_CV_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(38.5, 8.6)), module, Clonotribe::INPUT_GATE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(96.5, 8.6)), module, Clonotribe::INPUT_AUDIO_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(130.0, 8.6)), module, Clonotribe::INPUT_SYNC_CONNECTOR));

        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(21.0, 8.6)), module, Clonotribe::OUTPUT_CV_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(55.0, 8.6)), module, Clonotribe::OUTPUT_GATE_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(112.0, 8.6)), module, Clonotribe::OUTPUT_AUDIO_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(146.0, 8.6)), module, Clonotribe::OUTPUT_SYNC_CONNECTOR));
        
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.8, 77.0)), module, Clonotribe::LIGHT_SYNTH));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.5, 77.0)), module, Clonotribe::LIGHT_BASSDRUM));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(34.0, 77.0)), module, Clonotribe::LIGHT_SNARE));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(43.8, 77.0)), module, Clonotribe::LIGHT_HIGHHAT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(72.7, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_1));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(82.4, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_2));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.0, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_3));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(101.6, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_4));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(111.4, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_5));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.0, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_6));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(130.6, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_7));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.3, 77.0)), module, Clonotribe::LIGHT_SEQUENCER_8));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.9, 95.5)), module, Clonotribe::LIGHT_FLUX));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(24.4, 95.5)), module, Clonotribe::LIGHT_REC));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(39.0, 95.5)), module, Clonotribe::LIGHT_PLAY));

        RibbonWidget* ribbonWidget = new RibbonWidget();
        ribbonWidget->setModule(module);
        ribbonWidget->box.pos = mm2px(Vec(73.8, 99.2)); 
        addChild(ribbonWidget);
    }
    
    void onHoverKey(const event::HoverKey& e) override {
        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
            if (!clonotribeModule) return;
            
            switch (e.key) {
                case GLFW_KEY_F7: // ACTIVE STEP
                    if (clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(1.f);
                        e.consume(this);
                    }
                    break;
                    
                case GLFW_KEY_F8: // GATE TIME - trigger press
                    if (clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(1.f);
                        e.consume(this);
                    }
                    break;
            }
        }
        else if (e.action == GLFW_RELEASE) {
            Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
            if (!clonotribeModule) return;
            
            switch (e.key) {
                case GLFW_KEY_F7: // ACTIVE STEP release
                    clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(0.f);
                    e.consume(this);
                    break;
                    
                case GLFW_KEY_F8: // GATE TIME release
                    clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(0.f);
                    e.consume(this);
                    break;
            }
        }
        
        ModuleWidget::onHoverKey(e);
    }
};


Model* modelClonotribe = createModel<Clonotribe, ClonotribeWidget>("Clonotribe");

void Clonotribe::handleStepButtons() {
    for (int i = 0; i < 8; i++) {
        bool stepPressed = stepTriggers[i].process(params[PARAM_SEQUENCER_1_BUTTON + i].getValue() > 0.5f);
        if (stepPressed) {
            selectedStepForEditing = i;
            toggleStepInCurrentMode(i);
        }
    }
}

void Clonotribe::handleActiveStep() {
    bool activeStepHeld = params[PARAM_ACTIVE_STEP_BUTTON].getValue() > 0.5f;
    
    if (activeStepHeld && !activeStepWasPressed) {
        activeStepWasPressed = true;
        activeStepActive = true;
        
        // Copy current state
        for (int i = 0; i < 8; i++) {
            activeStepsSequencerSteps[i] = sequencer.isStepActive(i);
        }
        for (int d = 0; d < 3; d++) {
            for (int i = 0; i < 8; i++) {
                activeStepsDrumPatterns[d][i] = drumPatterns[d][i];
            }
        }
        
        // Toggle the selected step
        if (selectedStepForEditing >= 0 && selectedStepForEditing < 8) {
            if (selectedDrumPart == 0) {
                activeStepsSequencerSteps[selectedStepForEditing] = !activeStepsSequencerSteps[selectedStepForEditing];
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
    static float rollTimer = 0.f;
    
    if (gateTimeHeld && ribbon.touching && selectedDrumPart > 0) {
        float rollIntensity = ribbon.getDrumRollIntensity();
        float rollRate = rollIntensity * 50.f + 1.f; // 1-51 Hz roll rate
        rollTimer += args.sampleTime * rollRate;
        
        if (rollTimer >= 1.f) {
            rollTimer -= 1.f;
            switch (selectedDrumPart) {
                case 1: kickDrum.trigger(); break;
                case 2: snareDrum.trigger(); break;
                case 3: hiHat.trigger(); break;
            }
        }
    } else {
        rollTimer = 0.f;
    }
}

void Clonotribe::updateStepLights(const clonotribe::Sequencer::SequencerOutput& seqOutput) {
    for (int i = 0; i < 8; i++) {
        float brightness = 0.f;
        
        // Current playing step gets full brightness
        if (sequencer.playing && seqOutput.step == i) {
            brightness = 1.f;
        } else {
            bool stepActive = isStepActiveInCurrentMode(i);
            brightness = stepActive ? 0.3f : 0.f;
            
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
        
        lights[LIGHT_SEQUENCER_1 + i].setBrightness(brightness);
    }
}

bool Clonotribe::isStepActiveInCurrentMode(int step) {
    if (activeStepActive) {
        if (selectedDrumPart == 0) {
            return activeStepsSequencerSteps[step];
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? activeStepsDrumPatterns[drumIndex][step] : false;
        }
    } else {
        if (selectedDrumPart == 0) {
            return sequencer.isStepActive(step);
        } else {
            int drumIndex = selectedDrumPart - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    }
}

void Clonotribe::toggleStepInCurrentMode(int step) {
    if (activeStepActive) {
        if (selectedDrumPart == 0) {
            activeStepsSequencerSteps[step] = !activeStepsSequencerSteps[step];
        } else {
            int drumIndex = selectedDrumPart - 1;
            if (drumIndex >= 0 && drumIndex < 3) {
                activeStepsDrumPatterns[drumIndex][step] = !activeStepsDrumPatterns[drumIndex][step];
            }
        }
    } else {
        if (selectedDrumPart == 0) {
            sequencer.setStepActive(step, !sequencer.isStepActive(step));
        } else {
            int drumIndex = selectedDrumPart - 1;
            if (drumIndex >= 0 && drumIndex < 3) {
                drumPatterns[drumIndex][step] = !drumPatterns[drumIndex][step];
            }
        }
    }
}