#include "clonotribe.hpp"
#include "dsp/drumkits/original/kickdrum.hpp"
#include "dsp/drumkits/original/snaredrum.hpp"
#include "dsp/drumkits/original/hihat.hpp"
#include "dsp/drumkits/tr808/kickdrum.hpp"
#include "dsp/drumkits/tr808/snaredrum.hpp"
#include "dsp/drumkits/tr808/hihat.hpp"
#include "dsp/drumkits/latin/kickdrum.hpp"
#include "dsp/drumkits/latin/snaredrum.hpp"
#include "dsp/drumkits/latin/hihat.hpp"

void Clonotribe::cycleStepAccentGlide(int step) {
    if (step < 0 || step >= sequencer.getStepCount()) return;
    
    bool accent = sequencer.isStepAccent(step);
    bool glide = sequencer.isStepGlide(step);
    
    if (!accent && !glide) {
        sequencer.setStepAccent(step, true);
        sequencer.setStepGlide(step, false);
    } else if (accent && !glide) {
        sequencer.setStepAccent(step, false);
        sequencer.setStepGlide(step, true);
    } else if (!accent && glide) {
        sequencer.setStepAccent(step, true);
        sequencer.setStepGlide(step, true);
    } else {
        sequencer.setStepAccent(step, false);
        sequencer.setStepGlide(step, false);
    }
}

void Clonotribe::toggleActiveStep(int step) {
    int idx = step;
    if (sequencer.isInSixteenStepMode()) {
        idx = sequencer.getStepIndex(step, false);
    }
    if (idx >= 0 && idx < sequencer.getStepCount()) {
        sequencer.toggleStepSkipped(idx);
    }
}


void Clonotribe::handleStepButtons(float sampleTime) {
    for (int i = 0; i < 8; ++i) {
        float btnVal = params[PARAM_SEQUENCER_1_BUTTON + i].getValue();
        bool rising = (stepPrevVal[i] <= 0.5f && btnVal > 0.5f);

        if (rising) {
            bool isCtrlClick = (btnVal >= 0.85f && btnVal < 0.95f);
            
            int idx = i;
            if (sequencer.isInSixteenStepMode()) idx = sequencer.getStepIndex(i, false);
            
            if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH && idx >= 0 && idx < sequencer.getStepCount()) {
                if (isCtrlClick) {
                    cycleStepAccentGlide(idx);
                } else if (activeStepActive) {
                    sequencer.toggleStepSkipped(idx);
                } else {
                    sequencer.toggleStepMuted(idx);
                }
                selectedStepForEditing = i;
            } else if (sequencer.getSelectedDrumPart() != DrumPart::SYNTH) {
                int drumIdx = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
                if (drumIdx >= 0 && drumIdx < 3) {
                    drumPatterns[drumIdx][i] = !drumPatterns[drumIdx][i];
                }
            }
        }

        stepPrevVal[i] = btnVal;
    }
}

void Clonotribe::toggleStepInCurrentMode(int step) {
    if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH) {
        int idx = step;
        if (sequencer.isInSixteenStepMode()) {
            idx = sequencer.getStepIndex(step, false);
        }
        if (idx >= 0 && idx < sequencer.getStepCount()) {
            bool current = sequencer.isStepMuted(idx);
            sequencer.setStepMuted(idx, !current);
        }
    } else {
        int drumIdx = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
        if (drumIdx >= 0 && drumIdx < 3 && step >= 0 && step < 8) {
            drumPatterns[drumIdx][step] = !drumPatterns[drumIdx][step];
        }
    }
}

void Clonotribe::updateStepLights(const clonotribe::Sequencer::SequencerOutput& seqOutput) {
    for (int i = 0; i < 8; ++i) {
        int mainIdx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(i, false) : i;
        int base = LIGHT_SEQUENCER_1_R + i * 3;
        
        if (activeStepActive) {
            bool notSkipped = (mainIdx >= 0 && mainIdx < sequencer.getStepCount()) && !sequencer.steps[mainIdx].skipped;
            lights[base + 0].setBrightness(notSkipped ? 1.0f : 0.0f);
            lights[base + 1].setBrightness(0.0f);
            lights[base + 2].setBrightness(0.0f);
        } else {
            bool isPlaying = false;
            bool notMuted = false;
            
            if (sequencer.getSelectedDrumPart() == DrumPart::SYNTH) {
                notMuted = !sequencer.isStepMuted(mainIdx);
                isPlaying = sequencer.playing && (seqOutput.step == mainIdx);
                
                float baseBrightness = 0.0f;
                if (notMuted) baseBrightness = isPlaying ? 1.0f : 0.3f;
                
                float red = baseBrightness, green = 0.0f, blue = 0.0f;
                
                if (notMuted) {
                    bool accent = sequencer.isStepAccent(mainIdx);
                    bool glide = sequencer.isStepGlide(mainIdx);
                    
                    if (accent && glide) {
                        red = green = blue = baseBrightness;
                    } else if (accent) {
                        red = 0.0f;
                        green = baseBrightness;
                        blue = baseBrightness;
                    } else if (glide) {
                        red = 0.0f;
                        green = 0.0f;
                        blue = baseBrightness;
                    }
                }
                
                lights[base + 0].setBrightness(red);
                lights[base + 1].setBrightness(green);
                lights[base + 2].setBrightness(blue);
            } else {
                int drumIdx = static_cast<int>(sequencer.getSelectedDrumPart()) - 1;
                notMuted = (drumIdx >= 0 && drumIdx < 3) ? drumPatterns[drumIdx][i] : false;
                isPlaying = sequencer.playing && (seqOutput.step == i);
                
                float brightness = notMuted ? (isPlaying ? 1.0f : 0.3f) : 0.0f;
                lights[base + 0].setBrightness(brightness);
                lights[base + 1].setBrightness(0.0f);
                lights[base + 2].setBrightness(0.0f);
            }
        }
    }
}

Clonotribe::Clonotribe() : filterProcessor(vcf), ribbonController(this) {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    configSwitch(PARAM_VCO_WAVEFORM_SWITCH, 0.0f, 2.0f, 0.0f, "VCO Waveform", {"Square", "Triangle", "Sawtooth"});
    configSwitch(PARAM_RIBBON_RANGE_SWITCH, 0.0f, 2.0f, 0.0f, "Ribbon Controller Range", {"Key", "Narrow", "Wide"});
    configSwitch(PARAM_ENVELOPE_FORM_SWITCH, 0.0f, 2.0f, 0.0f, "Envelope",  {"Attack", "Gate", "Decay"});
    configSwitch(PARAM_LFO_TARGET_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Target",  {"VCF", "VCO+VCF", "VCO"});
    configSwitch(PARAM_LFO_MODE_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Mode", {"1 Shot", "Slow", "Fast"});
    configSwitch(PARAM_LFO_WAVEFORM_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Waveform",  {"Square", "Triangle", "Sawtooth"});
    configParam(PARAM_VCO_OCTAVE_KNOB, 0.0f, 5.0f, 2.0f, "VCO Octave");
    getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->snapEnabled = true;
    configParam(PARAM_NOISE_KNOB, 0.0f, 1.0f, 0.0f, "Noise Level");
    configParam(PARAM_VCF_CUTOFF_KNOB, 0.0f, 1.0f, 0.7f, "VCF Cutoff");
    configParam(PARAM_VCF_PEAK_KNOB, 0.0f, 1.0f, 0.1f, "VCF Peak (Resonance)");
    configParam(PARAM_VCA_LEVEL_KNOB, 0.0f, 1.0f, 0.8f, "VCA Level");
    configParam(PARAM_LFO_RATE_KNOB, 0.0f, 1.0f, 0.0f, "LFO Rate");
    configParam(PARAM_LFO_INTERVAL_KNOB, 0.0f, 1.0f, 0.0f, "LFO Intensity");
    configParam(PARAM_RHYTHM_VOLUME_KNOB, 0.0f, 1.0f, 0.0f, "Rhythm Volume");
    configParam(PARAM_SEQUENCER_TEMPO_KNOB, 0.0f, 1.0f, 0.5f, "Sequencer Tempo", " BPM", 0.0f, 600.0f, 10.0f);
    configParam(PARAM_DISTORTION_KNOB, 0.0f, 1.0f, 0.0f, "Distortion");
    configParam(PARAM_DELAY_TIME_KNOB, 0.0f, 1.0f, 0.0f, "Delay Time", " s", 0.0f, 1.99f, 0.01f);
    configParam(PARAM_DELAY_AMOUNT_KNOB, 0.0f, 1.0f, 0.0f, "Delay Amount");
    configParam(PARAM_ACCENT_GLIDE_KNOB, 0.0f, 1.0f, 0.0f, "Accent/Glide Amount");
    configButton(PARAM_SNARE_BUTTON, "Snare (F7)");
    configButton(PARAM_FLUX_BUTTON, "Flux (Tab)");
    configButton(PARAM_REC_BUTTON, "Record (Shift)");
    configButton(PARAM_SYNTH_BUTTON, "Synth (F5)");
    configButton(PARAM_BASSDRUM_BUTTON, "Bass Drum (F6)");
    configButton(PARAM_HIGHHAT_BUTTON, "Hi-Hat (F8)");
    configButton(PARAM_ACTIVE_STEP_BUTTON, "Active Step (F9)");
    configButton(PARAM_SEQUENCER_1_BUTTON, "Sequencer 1");
    configButton(PARAM_SEQUENCER_2_BUTTON, "Sequencer 2");
    configButton(PARAM_SEQUENCER_3_BUTTON, "Sequencer 3");
    configButton(PARAM_SEQUENCER_4_BUTTON, "Sequencer 4");
    configButton(PARAM_SEQUENCER_5_BUTTON, "Sequencer 5");
    configButton(PARAM_SEQUENCER_6_BUTTON, "Sequencer 6");
    configButton(PARAM_SEQUENCER_7_BUTTON, "Sequencer 7");
    configButton(PARAM_SEQUENCER_8_BUTTON, "Sequencer 8");

    for (int i = 0; i < 8; ++i) {
        if (auto* q = getParamQuantity(PARAM_SEQUENCER_1_BUTTON + i)) {
            q->snapEnabled = false;
        }
    }
    configButton(PARAM_PLAY_BUTTON, "Play (Space)");
    configButton(PARAM_GATE_TIME_BUTTON, "Gate Time (F10)");
    configInput(INPUT_CV_CONNECTOR, "CV VCO");
    configInput(INPUT_GATE_CONNECTOR, "Gate VCO");
    configInput(INPUT_AUDIO_CONNECTOR, "Audio");
    configInput(INPUT_SYNC_CONNECTOR, "Sync (Clock)");
    configInput(INPUT_VCO_OCTAVE_CONNECTOR, "Octave (CV)");
    configInput(INPUT_VCF_CUTOFF_CONNECTOR, "VCF Cutoff (CV)");
    configInput(INPUT_VCF_PEAK_CONNECTOR, "VCF Peak (CV)");
    configInput(INPUT_VCA_CONNECTOR, "VCA (CV)");
    configInput(INPUT_LFO_RATE_CONNECTOR, "LFO Rate (Clock)");
    configInput(INPUT_LFO_INTENSITY_CONNECTOR, "LFO Intensity (CV)");
    configInput(INPUT_DISTORTION_CONNECTOR, "Distortion (CV)");
    configInput(INPUT_DELAY_TIME_CONNECTOR, "Delay Clock (Clock)");
    configInput(INPUT_DELAY_AMOUNT_CONNECTOR, "Delay Amount (CV)");
    configInput(INPUT_NOISE_CONNECTOR, "Noise (CV)");
    configInput(INPUT_ACCENT_GLIDE_CONNECTOR, "Accent/Glide (CV)");
    configOutput(OUTPUT_CV_CONNECTOR, "CV");
    configOutput(OUTPUT_GATE_CONNECTOR, "Gate");
    configOutput(OUTPUT_AUDIO_CONNECTOR, "Audio");
    configOutput(OUTPUT_SYNC_CONNECTOR, "Sync (Clock)");
    configOutput(OUTPUT_SYNTH_CONNECTOR, "Synth");
    configOutput(OUTPUT_BASSDRUM_CONNECTOR, "Bass Drum");
    configOutput(OUTPUT_SNARE_CONNECTOR, "Snare");
    configOutput(OUTPUT_HIHAT_CONNECTOR, "Hi-Hat");
    configOutput(OUTPUT_LFO_RATE_CONNECTOR, "LFO Rate (Clock)");
    noiseGenerator.setNoiseType(NoiseType::WHITE);
    filterProcessor.setPointers(&vcf, &ladderFilter, &moogFilter);
    filterProcessor.setType(selectedFilterType);
    delayProcessor.clear();
    
    dcBlocker.setSampleRate(APP->engine->getSampleRate());
    dcBlocker.setCutoff(60.0f);
    dcBlocker.reset();
}

void Clonotribe::process(const ProcessArgs& args) {
    auto [cutoff, lfoIntensity, lfoRate, noiseLevel, resonance, rhythmVolume, tempo, volume, octave, distortion, envelopeType, lfoMode, lfoTarget, lfoWaveform, ribbonMode, waveform] = readParameters();
    
    updateDSPState(volume, rhythmVolume, lfoIntensity, ribbonMode, octave);
    handleMainTriggers();
    handleDrumSelectionAndTempo(tempo);
    handleActiveStep();
    handleStepButtons(args.sampleTime);
    bool gateTimeHeld = params[PARAM_GATE_TIME_BUTTON].getValue() > 0.5f;
    this->gateTimeHeld = gateTimeHeld;

    float ribbonGateTimeMod = gateTimesLocked ? 0.5f : (gateTimeHeld && ribbon.touching ? ribbon.getGateTimeMod() : 0.5f);

    handleDrumRolls(args, gateTimeHeld);

    float syncSignal = inputs[INPUT_SYNC_CONNECTOR].getVoltage();
    float cvVoltage = inputs[INPUT_CV_CONNECTOR].getVoltage();
    float inputPitch = cvVoltage + octave;
    float gate = inputs[INPUT_GATE_CONNECTOR].getVoltage();
    if (ribbon.touching) {
        inputPitch = ribbon.getCV();
        gate = ribbon.getGate();
    }

    bool cvGateTriggered = gateTrigger.process(gate > 1.0f);

    auto seqOutput = sequencer.process(args.sampleTime, inputPitch, gate, syncSignal, ribbonGateTimeMod, paramCache.accentGlideAmount);
    handleSequencerAndDrumState(seqOutput, inputPitch, gate, cvGateTriggered);

    const bool ribbonOverride = ribbon.touching;
    float finalPitch = ribbonOverride ? inputPitch : (sequencer.playing ? seqOutput.pitch : inputPitch);
    float finalGate = ribbonOverride ? gate : (sequencer.playing ? seqOutput.gate : gate);

    bool ribbonTriggered = ribbonGateTrigger.process(ribbon.getGate() > 1.0f);
    bool stepTrigger = sequencer.playing && seqOutput.stepChanged && seqOutput.gate > 1.0f;
    if (ribbonOverride) {
        if (ribbonTriggered) {
            envelope.trigger();
            gateActive = true;
        }
    } else {
        if (!sequencer.playing && cvGateTriggered) {
            envelope.trigger();
            gateActive = true;
        } else if (stepTrigger) {
            envelope.trigger();
            gateActive = true;
        }
    }
    if (finalGate < 0.5f && gateActive) {
        envelope.gateOff();
        gateActive = false;
    }

    float accentBoost = 0.0f;
    if (!ribbonOverride && seqOutput.accent && paramCache.accentGlideAmount > 0.0f) {
        accentBoost = 0.2f * paramCache.accentGlideAmount;
    }
    float effectiveCutoff = std::clamp(cutoff + accentBoost, 0.0f, 1.0f);

    float lfoFreq = rack::math::rescale(lfoRate, 0.0f, 1.0f, LFO::MIN_FREQ, LFO::MAX_FREQ);
    lfo.setRate(lfoFreq);
    LFO::Waveform wf = LFO::Waveform::SQUARE;
    switch (lfoWaveform) {
        case LFO::Waveform::SQUARE: wf = LFO::Waveform::SQUARE; break;
        case LFO::Waveform::TRIANGLE: wf = LFO::Waveform::TRIANGLE; break;
        case LFO::Waveform::SAW_TOOTH: wf = LFO::Waveform::SAW_TOOTH; break;
        default: wf = LFO::Waveform::SQUARE; break;
    }
    float lfoOut = lfo.process(args.sampleTime, wf) * lfoIntensity;

    float lfoToVCO = 0.0f, lfoToVCF = 0.0f;
    switch (lfoTarget) {
        case LFO::Target::VCF: lfoToVCF = lfoOut; break;
        case LFO::Target::VCO_VCF: lfoToVCO = lfoOut * 0.5f; lfoToVCF = lfoOut; break;
        case LFO::Target::VCO: lfoToVCO = lfoOut * 0.5f; break;
    }

    vco.setPitch(finalPitch + lfoToVCO);
    static VCO::Waveform cachedWaveform = static_cast<VCO::Waveform>(-1);
    static float (VCO::*vcoProcessFunction)(float) = nullptr;
    if (cachedWaveform != waveform) {
        switch (waveform) {
            case VCO::Waveform::SQUARE: vcoProcessFunction = &VCO::processSquare; break;
            case VCO::Waveform::TRIANGLE: vcoProcessFunction = &VCO::processTriangle; break;
            case VCO::Waveform::SAW: vcoProcessFunction = &VCO::processSaw; break;
            default: vcoProcessFunction = &VCO::processSquare; break;
        }
        cachedWaveform = waveform;
    }
    if (!vcoProcessFunction) {
        vcoProcessFunction = &VCO::processSquare;
    }
    
    float vcoOutput = (vco.*vcoProcessFunction)(args.sampleTime);
    float noise = noiseGenerator.process() * noiseLevel;
    float mixedSignal = vcoOutput + noise;
    
    float filteredSignal = filterProcessor.process(
        mixedSignal, 
        std::clamp(effectiveCutoff + lfoToVCF, 0.0f, 1.0f), 
        resonance
    );
    float envValue = processEnvelope(envelopeType, envelope, args.sampleTime, finalGate);
    float delayClock = inputs[INPUT_DELAY_TIME_CONNECTOR].isConnected() ? inputs[INPUT_DELAY_TIME_CONNECTOR].getVoltage() : 0.0f;
    float finalOutput = processOutput(
        filteredSignal, volume, envValue, ribbon.getVolumeAutomation(),
        rhythmVolume, args.sampleTime, noiseGenerator, seqOutput.step, distortion,
        delayClock, paramCache.delayTime, paramCache.delayAmount
    );

    if (outputs[OUTPUT_LFO_RATE_CONNECTOR].isConnected()) {
        static float prevPhase = 0.0f;
        if (lfo.phase < prevPhase) {
            lfoRatePulse.trigger(1e-3f);
        }
        prevPhase = lfo.phase;
        bool lfoClk = lfoRatePulse.process(args.sampleTime);
        outputs[OUTPUT_LFO_RATE_CONNECTOR].setVoltage(lfoClk ? 5.0f : 0.0f);
    } else {
        outputs[OUTPUT_LFO_RATE_CONNECTOR].setVoltage(0.0f);
    }
    
    if (distortion <= 0.1f) {
        finalOutput = clonotribe::FastMath::fastTanh(finalOutput * 0.7f) * 1.3f;
    }

    float noiseReducedOutput = dcBlocker.process(finalOutput);

    outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(std::clamp(noiseReducedOutput * 4.0f, -10.0f, 10.0f));
    outputs[OUTPUT_CV_CONNECTOR].setVoltage(finalPitch);
    outputs[OUTPUT_GATE_CONNECTOR].setVoltage(finalGate);
    bool syncOut = syncPulse.process(args.sampleTime);
    outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(syncOut ? 5.0f : 0.0f);

    lights[LIGHT_PLAY].setBrightness(sequencer.playing ? 1.0f : 0.0f);
    lights[LIGHT_REC].setBrightness(sequencer.recording ? 1.0f : 0.0f);
    lights[LIGHT_FLUX].setBrightness(sequencer.fluxMode ? 1.0f : 0.0f);
    lights[LIGHT_SYNTH].setBrightness(sequencer.getSelectedDrumPart() == DrumPart::SYNTH ? 1.0f : 0.0f);
    lights[LIGHT_BASSDRUM].setBrightness(sequencer.getSelectedDrumPart() == DrumPart::KICK ? 1.0f : 0.0f);
    lights[LIGHT_SNARE].setBrightness(sequencer.getSelectedDrumPart() == DrumPart::SNARE ? 1.0f : 0.0f);
    lights[LIGHT_HIGHHAT].setBrightness(sequencer.getSelectedDrumPart() == DrumPart::HIHAT ? 1.0f : 0.0f);

    updateStepLights(seqOutput);
}

struct TempoRangeItem : rack::MenuItem {
    Clonotribe* module;
    TempoRange range;
    void onAction(const rack::event::Action& e) override {
        module->selectedTempoRange = range;
        float min, max;
        module->getTempoRange(min, max);
        auto* q = module->getParamQuantity(Clonotribe::PARAM_SEQUENCER_TEMPO_KNOB);
        if (q) {
            q->name = "Sequencer Tempo";
            q->unit = " BPM";
            q->minValue = 0.0f;
            q->maxValue = 1.0f;
            q->displayBase = 0.0f;
            q->displayMultiplier = (max - min);
            q->displayOffset = min;
        }
    }
    void step() override {
        rightText = (module->selectedTempoRange == range) ? "✔" : "";
        MenuItem::step();
    }
};

struct DrumKitMenuItem : rack::MenuItem {
    Clonotribe* module;
    DrumKitType kitType;
    void onAction(const rack::event::Action& e) override {
        module->setDrumKit(kitType);
    }
    void step() override {
    static const char* kitLabels[static_cast<int>(DrumKitType::SIZE)] = {"Original", "TR-808", "Latin"};
    text = kitLabels[static_cast<int>(kitType)];
        rightText = (module->selectedDrumKit == kitType) ? "✔" : "";
        MenuItem::step();
    }
};

struct NoiseTypeMenuItem : rack::MenuItem {
    Clonotribe* module;
    NoiseType noiseType;
    void onAction(const rack::event::Action& e) override {
        module->setNoiseType(noiseType);
    }
    void step() override {
        static const char* noiseLabels[] = {"White", "Pink"};
        text = noiseLabels[static_cast<int>(noiseType)];
        rightText = (module->selectedNoiseType == noiseType) ? "✔" : "";
        MenuItem::step();
    }
};

struct FilterTypeMenuItem : rack::MenuItem {
    Clonotribe* module;
    FilterType filterType;
    void onAction(const rack::event::Action& e) override {
        module->setFilterType(filterType);
    }
    void step() override {
        static const char* filterLabels[] = {"Default (MS-20)", "Ladder (TB-303)", "Classic (Moog)"};
        text = filterLabels[static_cast<int>(filterType)];
        rightText = (module->selectedFilterType == filterType) ? "✔" : "";
        MenuItem::step();
    }
};

void Clonotribe::appendContextMenu(rack::ui::Menu* menu) {
    filterProcessor.setType(selectedFilterType);
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Filter Type"));
    for (int i = 0; i < static_cast<int>(FilterType::COUNT); ++i) {
        auto* filterItem = new FilterTypeMenuItem;
        filterItem->module = this;
        filterItem->filterType = static_cast<FilterType>(i);
        menu->addChild(filterItem);
    }
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Drum Kit"));
    for (int i = 0; i < static_cast<int>(DrumKitType::SIZE); ++i) {
        auto* kitItem = new DrumKitMenuItem;
        kitItem->module = this;
        kitItem->kitType = (DrumKitType)i;
        menu->addChild(kitItem);
    }
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Noise Type"));
    for (int i = 0; i < 2; ++i) {
        auto* noiseItem = new NoiseTypeMenuItem;
        noiseItem->module = this;
        noiseItem->noiseType = static_cast<NoiseType>(i);
        menu->addChild(noiseItem);
    }
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Tempo range"));
    static const char* rangeLabels[static_cast<int>(TempoRange::SIZE)] = {
        "10–600 BPM", "20–300 BPM", "60–180 BPM"
    };
    for (int i = 0; i < static_cast<int>(TempoRange::SIZE); ++i) {
        auto* item = new TempoRangeItem;
        item->module = this;
        item->range = (TempoRange)i;
        item->text = rangeLabels[i];
        menu->addChild(item);
    }

    struct SimpleActionItem : rack::MenuItem {
        std::function<void()> fn;
        void onAction(const rack::event::Action& e) override { if (fn) fn(); }
    };
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Utilities"));
    auto* clearAll = new SimpleActionItem();
    clearAll->text = "Clear All Sequences";
    clearAll->fn = [this]{ clearAllSequences(); };
    menu->addChild(clearAll);

    auto* clearSynth = new SimpleActionItem();
    clearSynth->text = "Clear Synth Sequence";
    clearSynth->fn = [this]{ clearSynthSequence(); };
    menu->addChild(clearSynth);

    auto* clearDrums = new SimpleActionItem();
    clearDrums->text = "Clear Drum Sequence";
    clearDrums->fn = [this]{ clearDrumSequence(); };
    menu->addChild(clearDrums);

    auto* enableActive = new SimpleActionItem();
    enableActive->text = "Enable All Active Steps";
    enableActive->fn = [this]{ enableAllActiveSteps(); };
    menu->addChild(enableActive);

    struct Toggle16 : rack::MenuItem { Clonotribe* module; void onAction(const rack::event::Action& e) override { module->sequencer.setSixteenStepMode(!module->sequencer.isInSixteenStepMode()); if (module->sequencer.currentStep >= module->sequencer.getStepCount()) module->sequencer.currentStep = 0; } void step() override { rightText = module->sequencer.isInSixteenStepMode()?"✔":""; MenuItem::step(); } };
    auto* t16 = new Toggle16();
    t16->module = this;
    t16->text = "16-step Mode";
    menu->addChild(t16);
}

json_t* Clonotribe::dataToJson() {
    json_t* rootJ = json_object();
    
    json_t* sequencerJ = json_object();
    
    json_object_set_new(sequencerJ, "sixteenStepMode", json_boolean(sequencer.isInSixteenStepMode()));
    json_object_set_new(sequencerJ, "currentStep", json_integer(sequencer.currentStep));
    json_object_set_new(sequencerJ, "recordingStep", json_integer(sequencer.recordingStep));
    json_object_set_new(sequencerJ, "fluxMode", json_boolean(sequencer.fluxMode));
    
    json_t* stepsJ = json_array();
    int stepCount = sequencer.getStepCount();
    for (int i = 0; i < stepCount; i++) {
        json_t* stepJ = json_object();
        json_object_set_new(stepJ, "pitch", json_real(sequencer.steps[i].pitch));
        json_object_set_new(stepJ, "gate", json_real(sequencer.steps[i].gate));
        json_object_set_new(stepJ, "gateTime", json_real(sequencer.steps[i].gateTime));
        json_object_set_new(stepJ, "skipped", json_boolean(sequencer.steps[i].skipped));
        json_object_set_new(stepJ, "muted", json_boolean(sequencer.steps[i].muted));
        json_object_set_new(stepJ, "accent", json_boolean(sequencer.steps[i].accent));
        json_object_set_new(stepJ, "glide", json_boolean(sequencer.steps[i].glide));
        json_array_append_new(stepsJ, stepJ);
    }
    json_object_set_new(sequencerJ, "steps", stepsJ);
    
    json_t* drumPatternsJ = json_array();
    for (int d = 0; d < 3; d++) {
        json_t* drumJ = json_array();
        for (int s = 0; s < 8; s++) {
            json_array_append_new(drumJ, json_boolean(drumPatterns[d][s]));
        }
        json_array_append_new(drumPatternsJ, drumJ);
    }
    json_object_set_new(sequencerJ, "drumPatterns", drumPatternsJ);
    
    json_object_set_new(rootJ, "sequencer", sequencerJ);
    
    json_object_set_new(rootJ, "selectedDrumPart", json_integer(static_cast<int>(sequencer.getSelectedDrumPart())));
    json_object_set_new(rootJ, "selectedStepForEditing", json_integer(selectedStepForEditing));
    json_object_set_new(rootJ, "gateTimesLocked", json_boolean(gateTimesLocked));
    json_object_set_new(rootJ, "lfoSampleAndHoldMode", json_boolean(lfoSampleAndHoldMode));
    json_object_set_new(rootJ, "syncHalfTempo", json_boolean(syncHalfTempo));
    json_object_set_new(rootJ, "selectedDrumKit", json_integer(static_cast<int>(selectedDrumKit)));
    json_object_set_new(rootJ, "selectedTempoRange", json_integer(static_cast<int>(selectedTempoRange)));
    json_object_set_new(rootJ, "selectedNoiseType", json_integer(static_cast<int>(selectedNoiseType)));
    
    return rootJ;
}

void Clonotribe::dataFromJson(json_t* rootJ) {
    json_t* sequencerJ = json_object_get(rootJ, "sequencer");
    if (sequencerJ) {
        json_t* sixteenStepModeJ = json_object_get(sequencerJ, "sixteenStepMode");
        if (sixteenStepModeJ) {
            sequencer.setSixteenStepMode(json_boolean_value(sixteenStepModeJ));
        }
        
        json_t* currentStepJ = json_object_get(sequencerJ, "currentStep");
        if (currentStepJ) {
            sequencer.currentStep = static_cast<int>(json_integer_value(currentStepJ));
        }
        
        json_t* recordingStepJ = json_object_get(sequencerJ, "recordingStep");
        if (recordingStepJ) {
            sequencer.recordingStep = static_cast<int>(json_integer_value(recordingStepJ));
        }
        
        json_t* fluxModeJ = json_object_get(sequencerJ, "fluxMode");
        if (fluxModeJ) {
            sequencer.fluxMode = json_boolean_value(fluxModeJ);
        }
        
        json_t* stepsJ = json_object_get(sequencerJ, "steps");
        if (stepsJ) {
            size_t stepIndex;
            json_t* stepJ;
            json_array_foreach(stepsJ, stepIndex, stepJ) {
                if (stepIndex < sequencer.kMaxSteps) {
                    json_t* pitchJ = json_object_get(stepJ, "pitch");
                    if (pitchJ) sequencer.steps[stepIndex].pitch = static_cast<float>(json_real_value(pitchJ));
                    
                    json_t* gateJ = json_object_get(stepJ, "gate");
                    if (gateJ) sequencer.steps[stepIndex].gate = static_cast<float>(json_real_value(gateJ));
                    
                    json_t* gateTimeJ = json_object_get(stepJ, "gateTime");
                    if (gateTimeJ) sequencer.steps[stepIndex].gateTime = static_cast<float>(json_real_value(gateTimeJ));
                    
                    json_t* skippedJ = json_object_get(stepJ, "skipped");
                    if (skippedJ) sequencer.steps[stepIndex].skipped = json_boolean_value(skippedJ);
                    
                    json_t* mutedJ = json_object_get(stepJ, "muted");
                    if (mutedJ) sequencer.steps[stepIndex].muted = json_boolean_value(mutedJ);
                        json_t* accentJ = json_object_get(stepJ, "accent");
                        if (accentJ) sequencer.steps[stepIndex].accent = json_boolean_value(accentJ);
                        json_t* glideJ = json_object_get(stepJ, "glide");
                        if (glideJ) sequencer.steps[stepIndex].glide = json_boolean_value(glideJ);
                }
            }
        }
        
        json_t* drumPatternsJ = json_object_get(sequencerJ, "drumPatterns");
        if (drumPatternsJ) {
            size_t drumIndex;
            json_t* drumJ;
            json_array_foreach(drumPatternsJ, drumIndex, drumJ) {
                if (drumIndex < 3) {
                    size_t stepIndex;
                    json_t* stepJ;
                    json_array_foreach(drumJ, stepIndex, stepJ) {
                        if (stepIndex < 8) {
                            drumPatterns[drumIndex][stepIndex] = json_boolean_value(stepJ);
                        }
                    }
                }
            }
        }
    }
    
    json_t* selectedDrumPartJ = json_object_get(rootJ, "selectedDrumPart");
    if (selectedDrumPartJ) {
        sequencer.setSelectedDrumPart(static_cast<DrumPart>(json_integer_value(selectedDrumPartJ)));
    }
    
    json_t* selectedStepForEditingJ = json_object_get(rootJ, "selectedStepForEditing");
    if (selectedStepForEditingJ) {
        selectedStepForEditing = static_cast<int>(json_integer_value(selectedStepForEditingJ));
    }
    
    json_t* gateTimesLockedJ = json_object_get(rootJ, "gateTimesLocked");
    if (gateTimesLockedJ) {
        gateTimesLocked = json_boolean_value(gateTimesLockedJ);
    }
    
    json_t* lfoSampleAndHoldModeJ = json_object_get(rootJ, "lfoSampleAndHoldMode");
    if (lfoSampleAndHoldModeJ) {
        lfoSampleAndHoldMode = json_boolean_value(lfoSampleAndHoldModeJ);
    }
    
    json_t* syncHalfTempoJ = json_object_get(rootJ, "syncHalfTempo");
    if (syncHalfTempoJ) {
        syncHalfTempo = json_boolean_value(syncHalfTempoJ);
    }
    
    json_t* selectedDrumKitJ = json_object_get(rootJ, "selectedDrumKit");
    if (selectedDrumKitJ) {
        setDrumKit(static_cast<DrumKitType>(json_integer_value(selectedDrumKitJ)));
    }
    
    json_t* selectedTempoRangeJ = json_object_get(rootJ, "selectedTempoRange");
    if (selectedTempoRangeJ) {
        selectedTempoRange = static_cast<TempoRange>(json_integer_value(selectedTempoRangeJ));
    }
    
    json_t* selectedNoiseTypeJ = json_object_get(rootJ, "selectedNoiseType");
    if (selectedNoiseTypeJ) {
        setNoiseType(static_cast<NoiseType>(json_integer_value(selectedNoiseTypeJ)));
    }
}

void Clonotribe::processBypass(const ProcessArgs& args) {
    if (inputs[INPUT_AUDIO_CONNECTOR].isConnected()) {
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(inputs[INPUT_AUDIO_CONNECTOR].getVoltage());
    } else {
        outputs[OUTPUT_AUDIO_CONNECTOR].setVoltage(0.0f);
    }
    
    if (inputs[INPUT_CV_CONNECTOR].isConnected()) {
        outputs[OUTPUT_CV_CONNECTOR].setVoltage(inputs[INPUT_CV_CONNECTOR].getVoltage());
    } else {
        outputs[OUTPUT_CV_CONNECTOR].setVoltage(0.0f);
    }
    
    if (inputs[INPUT_GATE_CONNECTOR].isConnected()) {
        outputs[OUTPUT_GATE_CONNECTOR].setVoltage(inputs[INPUT_GATE_CONNECTOR].getVoltage());
    } else {
        outputs[OUTPUT_GATE_CONNECTOR].setVoltage(0.0f);
    }
    
    if (inputs[INPUT_SYNC_CONNECTOR].isConnected()) {
        outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(inputs[INPUT_SYNC_CONNECTOR].getVoltage());
    } else {
        outputs[OUTPUT_SYNC_CONNECTOR].setVoltage(0.0f);
    }
    
    for (int i = 0; i < LIGHTS_LEN; i++) {
        lights[i].setBrightness(0.0f);
    }
}


void Clonotribe::onRandomize(const RandomizeEvent& e) {
    Module::onRandomize(e);
    
    int stepCount = sequencer.getStepCount();
    for (int i = 0; i < stepCount; i++) {
        sequencer.steps[i].pitch = rack::random::uniform() * 4.0f - 2.0f;
        sequencer.steps[i].gate = (rack::random::uniform() > 0.3f) ? 5.0f : 0.0f;
        sequencer.steps[i].gateTime = 0.1f + rack::random::uniform() * 0.8f;
        
        bool chance = (rack::random::uniform() < 0.2f);
        sequencer.steps[i].muted = chance;
        sequencer.steps[i].accent = chance;
        sequencer.steps[i].glide = chance;

        sequencer.steps[i].skipped = false;
    }
    
    for (int drum = 0; drum < 3; drum++) {
        for (int step = 0; step < 8; step++) {
            float probability;
            switch (drum) {
                case 0:
                    probability = ((step % 4) == 0 || (step % 4) == 2) ? 0.8f : 0.3f;
                    break;
                case 1:
                    probability = ((step % 4) == 1 || (step % 4) == 3) ? 0.7f : 0.2f;
                    break;
                case 2:
                    probability = 0.6f;
                    break;
                default:
                    probability = 0.5f;
                    break;
            }
            
            drumPatterns[drum][step] = (rack::random::uniform() < probability);
        }
    }
    
    if (rack::random::uniform() < 0.3f) {
        sequencer.setSixteenStepMode(true);
    } else {
        sequencer.setSixteenStepMode(false);
    }
    
    sequencer.fluxMode = (rack::random::uniform() < 0.2f);
    
    int randomKit = static_cast<int>(rack::random::uniform() * static_cast<float>(DrumKitType::SIZE));
    setDrumKit(static_cast<DrumKitType>(randomKit));

    int randomFilter = static_cast<int>(rack::random::uniform() * static_cast<float>(FilterType::COUNT));
    setFilterType(static_cast<FilterType>(randomFilter));
}