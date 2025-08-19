#include "../clonotribe.hpp"
#include <tuple>

auto Clonotribe::readParameters() -> std::tuple<float, float, float, float, float, float, float, float, float, float, EnvelopeType, SequencerStateManager::LFOMode, SequencerStateManager::LFOTarget, SequencerStateManager::LFOWaveform, int, int> {
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
        paramCache.delayTime = params[PARAM_DELAY_TIME_KNOB].getValue();
        paramCache.delayAmount = getParamWithCV(PARAM_DELAY_AMOUNT_KNOB, INPUT_DELAY_AMOUNT_CONNECTOR);
        paramCache.accentGlideAmount = getParamWithCV(PARAM_ACCENT_GLIDE_KNOB, INPUT_ACCENT_GLIDE_CONNECTOR);
        
        float octaveSwitch = params[PARAM_VCO_OCTAVE_KNOB].getValue();
        if (inputs[INPUT_VCO_OCTAVE_CONNECTOR].isConnected()) {
            float cvVoltage = inputs[INPUT_VCO_OCTAVE_CONNECTOR].getVoltage();
            octaveSwitch = std::clamp((cvVoltage + 5.0f) * 0.5f, 0.0f, 5.0f);
            getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->setDisplayValue(octaveSwitch);
        }
        paramCache.octave = octaveSwitch - 3.0f;
        paramCache.envelopeType = static_cast<EnvelopeType>(params[PARAM_ENVELOPE_FORM_SWITCH].getValue());
        paramCache.lfoMode = static_cast<SequencerStateManager::LFOMode>(params[PARAM_LFO_MODE_SWITCH].getValue());
        paramCache.lfoTarget = static_cast<SequencerStateManager::LFOTarget>(params[PARAM_LFO_TARGET_SWITCH].getValue());
        paramCache.lfoWaveform = static_cast<SequencerStateManager::LFOWaveform>(params[PARAM_LFO_WAVEFORM_SWITCH].getValue());
        paramCache.ribbonMode = static_cast<int>(params[PARAM_RIBBON_RANGE_SWITCH].getValue());
        paramCache.waveform = static_cast<int>(params[PARAM_VCO_WAVEFORM_SWITCH].getValue());
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
    (void)gateTimeHeld; // Gate Time no longer invokes combos; reserved for ribbon rolls only
}



void Clonotribe::handleActiveStep() {
    bool activeStepHeld = params[PARAM_ACTIVE_STEP_BUTTON].getValue() > 0.5f;

    if (activeStepHeld && !activeStepWasPressed) {
        activeStepWasPressed = true;
    activeStepActive = true; // enter skip-edit mode (no immediate toggle)
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