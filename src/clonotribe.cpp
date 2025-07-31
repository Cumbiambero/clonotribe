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

#include "clonotribe.hpp"

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
#include "clonotribe.hpp"

Clonotribe::Clonotribe() : ribbonController(this) {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
               
        configSwitch(PARAM_VCO_WAVEFORM_SWITCH, 0.0f, 2.0f, 0.0f, "VCO Waveform", {"Square", "Triangle", "Sawtooth"});
        configSwitch(PARAM_RIBBON_RANGE_SWITCH, 0.0f, 2.0f, 0.0f, "Ribbon Controller Range", {"Key", "Narrow", "Wide"});
        configSwitch(PARAM_ENVELOPE_FORM_SWITCH, 0.0f, 2.0f, 0.0f, "Envelope",  {"Attack", "Gate", "Decay"});
        configSwitch(PARAM_LFO_TARGET_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Target",  {"VCF", "VCO+VCF", "VCO"});
        configSwitch(PARAM_LFO_MODE_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Mode", {"1 Shot", "Slow", "Fast"});
        configSwitch(PARAM_LFO_WAVEFORM_SWITCH, 0.0f, 2.0f, 0.0f, "LFO Waveform",  {"Square", "Triangle", "Sawtooth"});
        
        configParam(PARAM_VCO_OCTAVE_KNOB, 0.0f, 5.0f, 2.0f, "VCO Octave");
        getParamQuantity(PARAM_VCO_OCTAVE_KNOB)->snapEnabled = true;

        configParam(PARAM_NOISE_KNOB, 0.0f, 1.0f, 0.0f, "Noise Level", "%", 0.0f, 100.0f);
        configParam(PARAM_VCF_CUTOFF_KNOB, 0.0f, 1.0f, 0.7f, "VCF Cutoff");
        configParam(PARAM_VCF_PEAK_KNOB, 0.0f, 1.0f, 0.0f, "VCF Peak (Resonance)");
        configParam(PARAM_VCA_LEVEL_KNOB, 0.0f, 1.0f, 0.8f, "VCA Level");
        configParam(PARAM_LFO_RATE_KNOB, 0.0f, 1.0f, 0.3f, "LFO Rate");
        configParam(PARAM_LFO_INTERVAL_KNOB, 0.0f, 1.0f, 0.0f, "LFO Intensity");
        configParam(PARAM_RHYTHM_VOLUME_KNOB, 0.0f, 1.0f, 0.0f, "Rhythm Volume");
        configParam(PARAM_SEQUENCER_TEMPO_KNOB, 0.0f, 1.0f, 0.5f, "Sequencer Tempo", " BPM", 0.0f, 600.0f, 10.0f);
        
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
        
        configInput(INPUT_CV_CONNECTOR, "CV");
        configInput(INPUT_GATE_CONNECTOR, "Gate");
        configInput(INPUT_AUDIO_CONNECTOR, "Audio");
        configInput(INPUT_SYNC_CONNECTOR, "Sync");
        
        configOutput(OUTPUT_CV_CONNECTOR, "CV");
        configOutput(OUTPUT_GATE_CONNECTOR, "Gate");
        configOutput(OUTPUT_AUDIO_CONNECTOR, "Audio");
        configOutput(OUTPUT_SYNC_CONNECTOR, "Sync");
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
        module->selectedDrumKit = kitType;
    }
    void step() override {
        static const char* kitLabels[DRUMKIT_COUNT] = {"Original", "TR-808", "Latin"};
        text = kitLabels[kitType];
        rightText = (module->selectedDrumKit == kitType) ? "✔" : "";
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