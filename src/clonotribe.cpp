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

void Clonotribe::toggleActiveStep(int step) {
    int idx = step;
    if (sequencer.isInSixteenStepMode()) {
        idx = sequencer.getStepIndex(step, false);
    }
    if (idx >= 0 && idx < sequencer.getStepCount()) {
        sequencer.toggleStepSkipped(idx);
    }
}


void Clonotribe::handleStepButtons() {
    for (int i = 0; i < 8; ++i) {
        if (stepTriggers[i].process(params[PARAM_SEQUENCER_1_BUTTON + i].getValue() > 0.5f)) {
            if (activeStepActive) {
                toggleActiveStep(i);
            } else {
                toggleStepInCurrentMode(i);
            }
        }
    }
}

void Clonotribe::toggleStepInCurrentMode(int step) {
    if (selectedDrumPart == 0) {
        int idx = step;
        if (sequencer.isInSixteenStepMode()) {
            idx = sequencer.getStepIndex(step, false);
        }
        if (idx >= 0 && idx < sequencer.getStepCount()) {
            bool current = sequencer.isStepMuted(idx);
            sequencer.setStepMuted(idx, !current);
        }
    } else {
        int drumIdx = selectedDrumPart - 1;
        if (drumIdx >= 0 && drumIdx < 3 && step >= 0 && step < 8) {
            drumPatterns[drumIdx][step] = !drumPatterns[drumIdx][step];
        }
    }
}

void Clonotribe::updateStepLights(const clonotribe::Sequencer::SequencerOutput& seqOutput) {
    for (int i = 0; i < 8; ++i) {
        if (activeStepActive) {
              int mainIdx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(i, false) : i;
            bool notSkipped = (mainIdx >= 0 && mainIdx < sequencer.getStepCount()) && !sequencer.steps[mainIdx].skipped;
            lights[LIGHT_SEQUENCER_1 + i].setBrightness(notSkipped ? 1.0f : 0.0f);
        } else {
            bool isPlaying = false;
            bool notMuted = false;
            if (selectedDrumPart == 0) {
                int mainIdx = sequencer.isInSixteenStepMode() ? sequencer.getStepIndex(i, false) : i;
                notMuted = !sequencer.isStepMuted(mainIdx);
                isPlaying = sequencer.playing && (seqOutput.step == mainIdx);
            } else {
                int drumIdx = selectedDrumPart - 1;
                notMuted = (drumIdx >= 0 && drumIdx < 3 && i >= 0 && i < 8) ? drumPatterns[drumIdx][i] : false;
                isPlaying = sequencer.playing && (seqOutput.step == i);
            }
            float brightness = 0.0f;
            if (isPlaying) {
                brightness = 1.0f;
            } else if (notMuted) {
                brightness = 0.3f;
            } else {
                brightness = 0.0f;
            }
            lights[LIGHT_SEQUENCER_1 + i].setBrightness(brightness);
        }
    }
}

Clonotribe::Clonotribe() : filterProcessor(vcf), ribbonController(this) {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        
        noiseGenerator.setNoiseType(NoiseType::WHITE);
               
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
        configParam(PARAM_VCF_PEAK_KNOB, 0.0f, 1.0f, 0.0f, "VCF Peak (Resonance)");
        configParam(PARAM_VCA_LEVEL_KNOB, 0.0f, 1.0f, 0.8f, "VCA Level");
        configParam(PARAM_LFO_RATE_KNOB, 0.0f, 1.0f, 0.0f, "LFO Rate");
        configParam(PARAM_LFO_INTERVAL_KNOB, 0.0f, 1.0f, 0.0f, "LFO Intensity");
        configParam(PARAM_RHYTHM_VOLUME_KNOB, 0.0f, 1.0f, 0.0f, "Rhythm Volume");
        configParam(PARAM_SEQUENCER_TEMPO_KNOB, 0.0f, 1.0f, 0.5f, "Sequencer Tempo", " BPM", 0.0f, 600.0f, 10.0f);
        configParam(PARAM_DISTORTION_KNOB, 0.0f, 1.0f, 0.0f, "Distortion");
        
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
        configInput(INPUT_NOISE_CONNECTOR, "Noise (CV)");

        configOutput(OUTPUT_CV_CONNECTOR, "CV");
        configOutput(OUTPUT_GATE_CONNECTOR, "Gate");
        configOutput(OUTPUT_AUDIO_CONNECTOR, "Audio");
        configOutput(OUTPUT_SYNC_CONNECTOR, "Sync (Clock)");
        configOutput(OUTPUT_SYNTH_CONNECTOR, "Synth");
        configOutput(OUTPUT_BASSDRUM_CONNECTOR, "Bass Drum");
        configOutput(OUTPUT_SNARE_CONNECTOR, "Snare");
        configOutput(OUTPUT_HIHAT_CONNECTOR, "Hi-Hat");
};

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
        static const char* kitLabels[DRUMKIT_COUNT] = {"Original", "TR-808", "Latin"};
        text = kitLabels[kitType];
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

void Clonotribe::appendContextMenu(rack::ui::Menu* menu) {
    menu->addChild(new rack::MenuSeparator());
    menu->addChild(rack::createMenuLabel("Drum Kit"));
    for (int i = 0; i < DRUMKIT_COUNT; ++i) {
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
    static const char* rangeLabels[TEMPO_RANGE_COUNT] = {
        "10–600 BPM", "20–300 BPM", "60–180 BPM"
    };
    for (int i = 0; i < TEMPO_RANGE_COUNT; ++i) {
        auto* item = new TempoRangeItem;
        item->module = this;
        item->range = (TempoRange)i;
        item->text = rangeLabels[i];
        menu->addChild(item);
    }
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
    
    json_object_set_new(rootJ, "selectedDrumPart", json_integer(selectedDrumPart));
    json_object_set_new(rootJ, "selectedStepForEditing", json_integer(selectedStepForEditing));
    json_object_set_new(rootJ, "gateTimesLocked", json_boolean(gateTimesLocked));
    json_object_set_new(rootJ, "lfoSampleAndHoldMode", json_boolean(lfoSampleAndHoldMode));
    json_object_set_new(rootJ, "syncHalfTempo", json_boolean(syncHalfTempo));
    json_object_set_new(rootJ, "selectedDrumKit", json_integer(selectedDrumKit));
    json_object_set_new(rootJ, "selectedTempoRange", json_integer(selectedTempoRange));
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
        selectedDrumPart = static_cast<int>(json_integer_value(selectedDrumPartJ));
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
        // Random pitch (-2 to +2 octaves)
        sequencer.steps[i].pitch = rack::random::uniform() * 4.0f - 2.0f;
        
        // Random gate (70% chance of being active)
        sequencer.steps[i].gate = (rack::random::uniform() > 0.3f) ? 5.0f : 0.0f;
        
        // Random gate time (0.1 to 0.9)
        sequencer.steps[i].gateTime = 0.1f + rack::random::uniform() * 0.8f;
        
        // Random muting (20% chance of being muted)
        sequencer.steps[i].muted = (rack::random::uniform() < 0.2f);
        
        // Don't skip steps in randomization (keep all steps active)
        sequencer.steps[i].skipped = false;
    }
    
    for (int drum = 0; drum < 3; drum++) {
        for (int step = 0; step < 8; step++) {
            float probability;
            switch (drum) {
                case 0: // Kick - more likely on beats 1 and 3
                    probability = ((step % 4) == 0 || (step % 4) == 2) ? 0.8f : 0.3f;
                    break;
                case 1: // Snare - more likely on beats 2 and 4
                    probability = ((step % 4) == 1 || (step % 4) == 3) ? 0.7f : 0.2f;
                    break;
                case 2: // Hi-hat - more evenly distributed
                    probability = 0.6f;
                    break;
                default:
                    probability = 0.5f;
            }
            
            drumPatterns[drum][step] = (rack::random::uniform() < probability);
        }
    }
    
    // Sometimes enable 16-step mode for more complex patterns
    if (rack::random::uniform() < 0.3f) {
        sequencer.setSixteenStepMode(true);
    } else {
        sequencer.setSixteenStepMode(false);
    }
    
    // Randomly enable flux mode (20% chance)
    sequencer.fluxMode = (rack::random::uniform() < 0.2f);
    
    // Randomly select a drum kit
    int randomKit = static_cast<int>(rack::random::uniform() * static_cast<float>(DRUMKIT_COUNT));
    setDrumKit(static_cast<DrumKitType>(randomKit));
}