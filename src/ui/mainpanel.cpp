
#include <rack.hpp>
#include "../clonotribe.hpp"
using namespace rack;
using namespace rack::app;
using namespace rack::componentlibrary;
using namespace rack::math;
using namespace rack::window;
using namespace rack::engine;
using namespace rack::ui;

struct MainPanel : ModuleWidget {
    ParamWidget* tempoKnob = nullptr;
    ParamWidget* lfoRateKnob = nullptr;
    ParamWidget* lfoModeSwitch = nullptr;
    bool keyboardShortcutsEnabled = false;

    TransparentButton* createButton(Vec pos, Vec size, Module* module, int paramId) {
        auto* button = createParam<TransparentButton>(pos, module, paramId);
        button->box.size = size;
        return button;
    }

    void setupParams(Clonotribe* module) {
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0f, 42.0f)), module, Clonotribe::PARAM_VCO_WAVEFORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0f, 62.5f)), module, Clonotribe::PARAM_RIBBON_RANGE_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(77.0f, 62.5f)), module, Clonotribe::PARAM_ENVELOPE_FORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(94.0f, 62.5f)), module, Clonotribe::PARAM_LFO_TARGET_SWITCH));
        
        lfoModeSwitch = createParamCentered<CKSSThree>(mm2px(Vec(109.0f, 62.5f)), module, Clonotribe::PARAM_LFO_MODE_SWITCH);
        addParam(lfoModeSwitch);
        
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(123.0f, 62.5f)), module, Clonotribe::PARAM_LFO_WAVEFORM_SWITCH));

        addParam(createParamCentered<OctaveKnob>(mm2px(Vec(15.0f, 42.0f)), module, Clonotribe::PARAM_VCO_OCTAVE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0f, 20.0f)), module, Clonotribe::PARAM_DISTORTION_KNOB)); 
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5f, 42.0f)), module, Clonotribe::PARAM_VCF_CUTOFF_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.0f, 42.0f)), module, Clonotribe::PARAM_VCA_LEVEL_KNOB));
        
        lfoRateKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(98.0f, 42.0f)), module, Clonotribe::PARAM_LFO_RATE_KNOB);
        addParam(lfoRateKnob);
        
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(119.0f, 42.0f)), module, Clonotribe::PARAM_LFO_INTERVAL_KNOB));        
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0f, 42.0f)), module, Clonotribe::PARAM_RHYTHM_VOLUME_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0f, 62.5f)), module, Clonotribe::PARAM_NOISE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5f, 62.5f)), module, Clonotribe::PARAM_VCF_PEAK_KNOB));
        
        tempoKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0f, 62.5f)), module, Clonotribe::PARAM_SEQUENCER_TEMPO_KNOB);
        addParam(tempoKnob);
    }

    void setupButtons(Clonotribe* module) {
        addParam(createButton(mm2px(Vec(30.25f, 103.25f)), Vec(51.0f, 18.0f), module, Clonotribe::PARAM_PLAY_BUTTON));
        addParam(createButton(mm2px(Vec(11.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SYNTH_BUTTON));
        addParam(createButton(mm2px(Vec(20.5f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_BASSDRUM_BUTTON));
        addParam(createButton(mm2px(Vec(30.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SNARE_BUTTON));
        addParam(createButton(mm2px(Vec(39.5f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_HIGHHAT_BUTTON));
        addParam(createButton(mm2px(Vec(11.0f, 103.25f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_FLUX_BUTTON));
        addParam(createButton(mm2px(Vec(20.5f, 103.25f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_REC_BUTTON));
        addParam(createButton(mm2px(Vec(68.5f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_1_BUTTON));
        addParam(createButton(mm2px(Vec(78.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_2_BUTTON));
        addParam(createButton(mm2px(Vec(88.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_3_BUTTON));
        addParam(createButton(mm2px(Vec(97.5f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_4_BUTTON));
        addParam(createButton(mm2px(Vec(107.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_5_BUTTON));
        addParam(createButton(mm2px(Vec(117.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_6_BUTTON));
        addParam(createButton(mm2px(Vec(126.5f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_7_BUTTON));
        addParam(createButton(mm2px(Vec(136.0f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_SEQUENCER_8_BUTTON));
        addParam(createButton(mm2px(Vec(49.25f, 84.5f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_ACTIVE_STEP_BUTTON));
        addParam(createButton(mm2px(Vec(49.25f, 103.25f)), Vec(24.0f, 18.0f), module, Clonotribe::PARAM_GATE_TIME_BUTTON));
    }

    void setupInputs(Clonotribe* module) {
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 20.0f)), module, Clonotribe::INPUT_AUDIO_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 30.0f)), module, Clonotribe::INPUT_CV_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 40.0f)), module, Clonotribe::INPUT_GATE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 50.0f)), module, Clonotribe::INPUT_SYNC_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 60.0f)), module, Clonotribe::INPUT_VCO_OCTAVE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 70.0f)), module, Clonotribe::INPUT_VCF_CUTOFF_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 80.0f)), module, Clonotribe::INPUT_VCF_PEAK_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 90.0f)), module, Clonotribe::INPUT_LFO_RATE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 100.0f)), module, Clonotribe::INPUT_LFO_INTENSITY_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 110.f)), module, Clonotribe::INPUT_DISTORTION_CONNECTOR));
    }

    void setupOutputs(Clonotribe* module) {
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(95.0f, 10.0f)), module, Clonotribe::OUTPUT_SYNTH_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(104.0f, 10.0f)), module, Clonotribe::OUTPUT_BASSDRUM_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(113.0f, 10.0f)), module, Clonotribe::OUTPUT_SNARE_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(122.0f, 10.0f)), module, Clonotribe::OUTPUT_HIHAT_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 20.0f)), module, Clonotribe::OUTPUT_AUDIO_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 30.0f)), module, Clonotribe::OUTPUT_CV_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 40.0f)), module, Clonotribe::OUTPUT_GATE_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 50.0f)), module, Clonotribe::OUTPUT_SYNC_CONNECTOR));        
    }

    void setupLights(Clonotribe* module) {

        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.8f, 77.0f)), module, Clonotribe::LIGHT_SYNTH));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.5f, 77.0f)), module, Clonotribe::LIGHT_BASSDRUM));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(34.0f, 77.0f)), module, Clonotribe::LIGHT_SNARE));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(43.8f, 77.0f)), module, Clonotribe::LIGHT_HIGHHAT));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(72.7f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_1));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(82.4f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_2));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(92.0f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_3));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(101.6f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_4));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(111.4f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_5));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(121.0f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_6));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(130.6f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_7));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(140.3f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_8));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.9f, 95.5f)), module, Clonotribe::LIGHT_FLUX));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(24.4f, 95.5f)), module, Clonotribe::LIGHT_REC));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(39.0f, 95.5f)), module, Clonotribe::LIGHT_PLAY));
    }

    MainPanel(Clonotribe* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));
        setupParams(module);
        setupButtons(module);
        setupInputs(module);
        setupOutputs(module);
        setupLights(module);
        addChild(new RibbonController(module));
    }

    void hideParamsForConnectedInputs() {
        Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
        if (!clonotribeModule) return;

        if (tempoKnob) {
            tempoKnob->visible = !clonotribeModule->inputs[Clonotribe::INPUT_SYNC_CONNECTOR].isConnected();
        }

        bool lfoRateConnected = clonotribeModule->inputs[Clonotribe::INPUT_LFO_RATE_CONNECTOR].isConnected();
        
        if (lfoRateKnob) {
            lfoRateKnob->visible = !lfoRateConnected;
        }
        
        if (lfoModeSwitch) {
            lfoModeSwitch->visible = !lfoRateConnected;
        }
    }

    void step() override {
        hideParamsForConnectedInputs();
        ModuleWidget::step();
    }

    void handleHoverKey(const event::HoverKey& e) {
        if (!keyboardShortcutsEnabled) {
            return;
        }
        
        Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
        if (!clonotribeModule) return;
        
        if (clonotribeModule->inputs[Clonotribe::INPUT_CV_CONNECTOR].isConnected() || 
            clonotribeModule->inputs[Clonotribe::INPUT_GATE_CONNECTOR].isConnected()) {
            keyboardShortcutsEnabled = false;
            e.consume(this);
            return; // Skip all keyboard handling when CV/GATE inputs are connected
        }
        
        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
            if (!clonotribeModule) return;
            switch (e.key) {
                case GLFW_KEY_F5:
                    if (clonotribeModule->params[Clonotribe::PARAM_SYNTH_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SYNTH_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_F6:
                    if (clonotribeModule->params[Clonotribe::PARAM_BASSDRUM_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_BASSDRUM_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_F7:
                    if (clonotribeModule->params[Clonotribe::PARAM_SNARE_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SNARE_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_F8:
                    if (clonotribeModule->params[Clonotribe::PARAM_HIGHHAT_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_HIGHHAT_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_F9:
                    if (clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_F10:
                    if (clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_TAB:
                    if (clonotribeModule->params[Clonotribe::PARAM_FLUX_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_FLUX_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    if (clonotribeModule->params[Clonotribe::PARAM_REC_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_REC_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_SPACE:
                    if (clonotribeModule->params[Clonotribe::PARAM_PLAY_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_PLAY_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_1:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_2:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_3:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_4:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_5:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_6:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_7:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].setValue(1.0f);
                    break;
                case GLFW_KEY_8:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].getValue() < 0.5f)
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].setValue(1.0f);
                    break;
            }
        } else if (e.action == GLFW_RELEASE) {
            switch (e.key) {
                case GLFW_KEY_F5:
                    clonotribeModule->params[Clonotribe::PARAM_SYNTH_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_F6:
                    clonotribeModule->params[Clonotribe::PARAM_BASSDRUM_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_F7:
                    clonotribeModule->params[Clonotribe::PARAM_SNARE_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_F8:
                    clonotribeModule->params[Clonotribe::PARAM_HIGHHAT_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_F9:
                    clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_F10:
                    clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_TAB:
                    clonotribeModule->params[Clonotribe::PARAM_FLUX_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    clonotribeModule->params[Clonotribe::PARAM_REC_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_SPACE:
                    clonotribeModule->params[Clonotribe::PARAM_PLAY_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_1:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_2:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_3:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_4:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_5:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_6:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_7:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].setValue(0.0f);
                    break;
                case GLFW_KEY_8:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].setValue(0.0f);
                    break;
            }
            e.consume(this);
        }
    }

    void onHoverKey(const event::HoverKey& e) override {
        handleHoverKey(e);
        ModuleWidget::onHoverKey(e);
    }

    void appendContextMenu(rack::ui::Menu* menu) override {
        ModuleWidget::appendContextMenu(menu);
        
        menu->addChild(new MenuSeparator);
        menu->addChild(createBoolPtrMenuItem("Keyboard Shortcuts", "", &keyboardShortcutsEnabled));
        
        Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
        if (clonotribeModule) {
            clonotribeModule->appendContextMenu(menu);
        }
    }
};


Model* modelClonotribe = createModel<Clonotribe, MainPanel>("Clonotribe");