#include "../clonotribe.hpp"
#include <tuple>

auto Clonotribe::readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, float, Envelope::Type, LFO::Mode, LFO::Target, LFO::Waveform, Ribbon::Mode, VCO::Waveform> {
    auto getParamWithCV = [this](int paramId, int inputId) -> float {
        if (paramCache.inputConnected[inputId]) {
            float cvVoltage = inputs[inputId].getVoltage();
            float value = std::clamp((cvVoltage + 5.0f) * 0.1f, ZERO, ONE);
            getParamQuantity(paramId)->setDisplayValue(value);
            return value;
        } else {
            return params[paramId].getValue();
        }
    };
    
    paramCache.cutoff = getParamWithCV(PARAM_VCF_CUTOFF_KNOB, INPUT_VCF_CUTOFF_CONNECTOR);
    paramCache.lfoIntensity = getParamWithCV(PARAM_LFO_INTENSITY_KNOB, INPUT_LFO_INTENSITY_CONNECTOR) * 10.f;
    paramCache.lfoRate = getParamWithCV(PARAM_LFO_RATE_KNOB, INPUT_LFO_RATE_CONNECTOR);
    paramCache.noiseLevel = getParamWithCV(PARAM_NOISE_KNOB, INPUT_NOISE_CONNECTOR);
    paramCache.resonance = getParamWithCV(PARAM_VCF_PEAK_KNOB, INPUT_VCF_PEAK_CONNECTOR);
    paramCache.volume = getParamWithCV(PARAM_VCA_LEVEL_KNOB, INPUT_VCA_CONNECTOR);
    paramCache.distortion = getParamWithCV(PARAM_DISTORTION_KNOB, INPUT_DISTORTION_CONNECTOR);
    paramCache.delayAmount = getParamWithCV(PARAM_DELAY_AMOUNT_KNOB, INPUT_DELAY_AMOUNT_CONNECTOR);
    paramCache.accentGlideAmount = getParamWithCV(PARAM_ACCENT_GLIDE_KNOB, INPUT_ACCENT_GLIDE_CONNECTOR);

    if (paramCache.needsUpdate()) {
        for (int i = 0; i < INPUTS_LEN; ++i) {
            paramCache.inputConnected[i] = inputs[i].isConnected();
        }

        float octaveSwitch = params[PARAM_VCO_OCTAVE_KNOB].getValue();
        if (paramCache.inputConnected[INPUT_VCO_OCTAVE_CONNECTOR]) {
            float cvVoltage = inputs[INPUT_VCO_OCTAVE_CONNECTOR].getVoltage();
            octaveSwitch = std::clamp((cvVoltage + 5.0f) * HALF, ZERO, 5.0f);
            getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->setDisplayValue(octaveSwitch);
        }

        paramCache.octave = octaveSwitch - 3.0f;
        paramCache.rhythmVolume = params[PARAM_RHYTHM_VOLUME_KNOB].getValue();
        paramCache.tempo = params[PARAM_SEQUENCER_TEMPO_KNOB].getValue();
        paramCache.delayTime = params[PARAM_DELAY_TIME_KNOB].getValue();
        paramCache.envelopeType = static_cast<Envelope::Type>(params[PARAM_ENVELOPE_FORM_SWITCH].getValue());
        paramCache.lfoMode = static_cast<LFO::Mode>(params[PARAM_LFO_MODE_SWITCH].getValue());
        paramCache.lfoTarget = static_cast<LFO::Target>(params[PARAM_LFO_TARGET_SWITCH].getValue());
        paramCache.lfoWaveform = static_cast<LFO::Waveform>(params[PARAM_LFO_WAVEFORM_SWITCH].getValue());
        paramCache.ribbonMode = static_cast<Ribbon::Mode>(params[PARAM_RIBBON_RANGE_SWITCH].getValue());
        paramCache.vcoWaveform = static_cast<VCO::Waveform>(params[PARAM_VCO_WAVEFORM_SWITCH].getValue());
        paramCache.resetUpdateCounter();
    }

    return {paramCache.cutoff, paramCache.lfoIntensity, paramCache.lfoRate, paramCache.noiseLevel, 
            paramCache.resonance, paramCache.rhythmVolume, paramCache.tempo, paramCache.volume, 
            paramCache.octave, paramCache.distortion, paramCache.envelopeType, paramCache.lfoMode, paramCache.lfoTarget, 
            paramCache.lfoWaveform, paramCache.ribbonMode, paramCache.vcoWaveform};
}

void Clonotribe::updateDSPState(float volume, float rhythmVolume, float lfoIntensity, Ribbon::Mode ribbonMode, float octave, float cutoff) {
    bool vcfActive = volume > MIN && cutoff > MIN;
    filterProcessor.setActive(vcfActive);
    lfo.setActive(vcfActive && lfoIntensity > MIN);
    ribbon.setMode(static_cast<int>(ribbonMode));
    ribbon.setOctave(octave);    
}

void Clonotribe::handleMainTriggers() {
    bool playPressed = playTrigger.process(params[PARAM_PLAY_BUTTON].getValue() > HALF);
    bool recPressed = recTrigger.process(params[PARAM_REC_BUTTON].getValue() > HALF);
    bool fluxPressed = fluxTrigger.process(params[PARAM_FLUX_BUTTON].getValue() > HALF);
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
        if (params[PARAM_PLAY_BUTTON].getValue() > HALF) {
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
    bool synthPressed = drumTriggers[0].process(params[PARAM_SYNTH_BUTTON].getValue() > HALF);
    bool kickPressed = drumTriggers[1].process(params[PARAM_BASSDRUM_BUTTON].getValue() > HALF);
    bool snarePressed = drumTriggers[2].process(params[PARAM_SNARE_BUTTON].getValue() > HALF);
    bool hihatPressed = drumTriggers[3].process(params[PARAM_HIGHHAT_BUTTON].getValue() > HALF);
    if (synthPressed) {
        sequencer.setSelectedDrumPart(DrumPart::SYNTH);
    }
    if (kickPressed) {
        sequencer.setSelectedDrumPart(DrumPart::KICK);
    }
    if (snarePressed) {
        sequencer.setSelectedDrumPart(DrumPart::SNARE);
    }
    if (hihatPressed) {
        sequencer.setSelectedDrumPart(DrumPart::HIHAT);
    }
    if (!inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
        float minTempo, maxTempo;
        getTempoRange(minTempo, maxTempo);
        float bpm = rack::math::rescale(tempo, ZERO, ONE, minTempo, maxTempo);
        sequencer.setTempo(bpm);
        sequencer.setExternalSync(false);
    } else {
        sequencer.setExternalSync(true);
    }
}

void Clonotribe::handleActiveStep() {
    bool activeStepHeld = params[PARAM_ACTIVE_STEP_BUTTON].getValue() > HALF;

    if (activeStepHeld && !activeStepWasPressed) {
        activeStepWasPressed = true;
        activeStepActive = true;
    } else if (!activeStepHeld && activeStepWasPressed) {
        activeStepWasPressed = false;
        activeStepActive = false;
    }
}

void Clonotribe::handleDrumRolls(const ProcessArgs& args, bool gateTimeHeld) {
    static float rollTimer = ZERO;

    DrumPart selectedPart = sequencer.getSelectedDrumPart();
    bool isDrumPart = (selectedPart != DrumPart::SYNTH);
    bool sequencerRunning = sequencer.playing;
    int currentStep = sequencer.currentStep;
    bool stepActive = isDrumPart && isStepActiveInCurrentMode(currentStep);

    if (gateTimeHeld && ribbon.touching && isDrumPart && sequencerRunning && stepActive) {
        float rollIntensity = ribbon.getDrumRollIntensity();
        float rollRate = rollIntensity * 50.0f + ONE;
        rollTimer += args.sampleTime * rollRate;
        
        if (rollTimer >= ONE) {
            rollTimer -= ONE;
            switch (selectedPart) {
                case DrumPart::KICK: triggerKick(); break;
                case DrumPart::SNARE: triggerSnare(); break;
                case DrumPart::HIHAT: triggerHihat(); break;
                case DrumPart::SYNTH: break;
            }
        }
    } else {
        rollTimer = ZERO;
    }
}

bool Clonotribe::isStepActiveInCurrentMode(int step) {
    if (activeStepActive) {
        if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH) {
            int idx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(step, false) : step;
            return (idx >= 0 && idx < sequencer.getStepCount()) && !sequencer.isStepSkipped(idx);
        } else {
            int drumIndex = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    } else {
        if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH) {
            int idx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(step, false) : step;
            return (idx >= 0 && idx < sequencer.getStepCount()) && !sequencer.isStepMuted(idx);
        } else {
            int drumIndex = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
            return (drumIndex >= 0 && drumIndex < 3) ? drumPatterns[drumIndex][step] : false;
        }
    }
}

void Clonotribe::clearAllSequences() {
    sequencer.clearSequence();
    clearDrumSequence();
}

void Clonotribe::clearSynthSequence() {
    sequencer.clearSequence();
}

void Clonotribe::clearDrumSequence() {
    for (int d = 0; d < 3; d++) {
        for (int s = 0; s < 8; s++) {
            drumPatterns[d][s] = false;
        }
    }
}

void Clonotribe::enableAllActiveSteps() {
    if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH) {
        sequencer.enableAllSteps();
    } else {
        int drumIndex = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
        if (drumIndex >= 0 && drumIndex < 3) {
            for (int s = 0; s < 8; s++) {
                drumPatterns[drumIndex][s] = true;
            }
        }
    }
}