
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
    ParamWidget* delayTimeKnob = nullptr;

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

        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.0f, 12.0f)), module, Clonotribe::PARAM_ACCENT_GLIDE_KNOB));
        delayTimeKnob = createParamCentered<RoundBlackKnob>(mm2px(Vec(98.0f, 12.0f)), module, Clonotribe::PARAM_DELAY_TIME_KNOB);
        addParam(delayTimeKnob);
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(119.0f, 12.0f)), module, Clonotribe::PARAM_DELAY_AMOUNT_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0f, 12.0f)), module, Clonotribe::PARAM_DISTORTION_KNOB));

        addParam(createParamCentered<OctaveKnob>(mm2px(Vec(15.0f, 42.0f)), module, Clonotribe::PARAM_VCO_OCTAVE_KNOB));
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
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(46.0f, 10.0f)), module, Clonotribe::INPUT_DELAY_TIME_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(55.0f, 10.0f)), module, Clonotribe::INPUT_DELAY_AMOUNT_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(64.0f, 10.0f)), module, Clonotribe::INPUT_ACCENT_GLIDE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 18.0f)), module, Clonotribe::INPUT_AUDIO_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 27.0f)), module, Clonotribe::INPUT_CV_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 36.0f)), module, Clonotribe::INPUT_GATE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 45.0f)), module, Clonotribe::INPUT_SYNC_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 54.0f)), module, Clonotribe::INPUT_LFO_RATE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 63.0f)), module, Clonotribe::INPUT_LFO_INTENSITY_CONNECTOR)); 
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 72.0f)), module, Clonotribe::INPUT_VCF_CUTOFF_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 81.0f)), module, Clonotribe::INPUT_VCF_PEAK_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 90.0f)), module, Clonotribe::INPUT_VCO_OCTAVE_CONNECTOR));
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 99.0f)), module, Clonotribe::INPUT_VCA_CONNECTOR));        
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 108.0f)), module, Clonotribe::INPUT_DISTORTION_CONNECTOR));        
        addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(172.0f, 117.0f)), module, Clonotribe::INPUT_NOISE_CONNECTOR));
    }

    void setupOutputs(Clonotribe* module) {
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.0f, 10.0f)), module, Clonotribe::OUTPUT_SYNTH_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(19.0f, 10.0f)), module, Clonotribe::OUTPUT_BASSDRUM_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(28.0f, 10.0f)), module, Clonotribe::OUTPUT_SNARE_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(37.0f, 10.0f)), module, Clonotribe::OUTPUT_HIHAT_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 18.0f)), module, Clonotribe::OUTPUT_AUDIO_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 27.0f)), module, Clonotribe::OUTPUT_CV_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 36.0f)), module, Clonotribe::OUTPUT_GATE_CONNECTOR));
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 45.0f)), module, Clonotribe::OUTPUT_SYNC_CONNECTOR));        
        addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(163.0f, 54.0f)), module, Clonotribe::OUTPUT_LFO_RATE_CONNECTOR));        
    }

    void setupLights(Clonotribe* module) {
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.8f, 77.0f)), module, Clonotribe::LIGHT_SYNTH));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(24.5f, 77.0f)), module, Clonotribe::LIGHT_BASSDRUM));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(34.0f, 77.0f)), module, Clonotribe::LIGHT_SNARE));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(43.8f, 77.0f)), module, Clonotribe::LIGHT_HIGHHAT));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(14.9f, 95.5f)), module, Clonotribe::LIGHT_FLUX));
        addChild(createLightCentered<MediumLight<GreenLight>>(mm2px(Vec(39.0f, 95.5f)), module, Clonotribe::LIGHT_PLAY));
        addChild(createLightCentered<MediumLight<RedLight>>(mm2px(Vec(24.4f, 95.5f)), module, Clonotribe::LIGHT_REC));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(72.7f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_1_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(82.4f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_2_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(92.0f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_3_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(101.6f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_4_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(111.4f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_5_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(121.0f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_6_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(130.6f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_7_R));
        addChild(createLightCentered<MediumLight<RedGreenBlueLight>>(mm2px(Vec(140.3f, 77.0f)), module, Clonotribe::LIGHT_SEQUENCER_8_R));
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

        if (delayTimeKnob) {
            delayTimeKnob->visible = !clonotribeModule->inputs[Clonotribe::INPUT_DELAY_TIME_CONNECTOR].isConnected();
        }
    }

    void step() override {
        hideParamsForConnectedInputs();
        ModuleWidget::step();
    }

    void handleHoverKey(const event::HoverKey& e) {
        Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
        if (!clonotribeModule) return;

        auto setStepParam = [&](int paramId, bool ctrlPressed, bool stepPressed) {
            float result = stepPressed ? (ctrlPressed ? KEY_PRESSED_WITH_CTRL : KEY_PRESSED_NORMAL) : ZERO;
            clonotribeModule->params[paramId].setValue(result);
        };

        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            bool ctrl = (APP->window->getMods() & RACK_MOD_CTRL) != 0;
            switch (e.key) {
                case GLFW_KEY_F5: clonotribeModule->params[Clonotribe::PARAM_SYNTH_BUTTON].setValue(ONE); break;
                case GLFW_KEY_F6: clonotribeModule->params[Clonotribe::PARAM_BASSDRUM_BUTTON].setValue(ONE); break;
                case GLFW_KEY_F7: clonotribeModule->params[Clonotribe::PARAM_SNARE_BUTTON].setValue(ONE); break;
                case GLFW_KEY_F8: clonotribeModule->params[Clonotribe::PARAM_HIGHHAT_BUTTON].setValue(ONE); break;
                case GLFW_KEY_F9: clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(ONE); break;
                case GLFW_KEY_F10: clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(ONE); break;
                case GLFW_KEY_TAB: clonotribeModule->params[Clonotribe::PARAM_FLUX_BUTTON].setValue(ONE); break;
                case GLFW_KEY_LEFT_SHIFT: clonotribeModule->params[Clonotribe::PARAM_REC_BUTTON].setValue(ONE); break;
                case GLFW_KEY_SPACE: clonotribeModule->params[Clonotribe::PARAM_PLAY_BUTTON].setValue(ONE); break;
                case GLFW_KEY_1: setStepParam(Clonotribe::PARAM_SEQUENCER_1_BUTTON, ctrl, true); break;
                case GLFW_KEY_2: setStepParam(Clonotribe::PARAM_SEQUENCER_2_BUTTON, ctrl, true); break;
                case GLFW_KEY_3: setStepParam(Clonotribe::PARAM_SEQUENCER_3_BUTTON, ctrl, true); break;
                case GLFW_KEY_4: setStepParam(Clonotribe::PARAM_SEQUENCER_4_BUTTON, ctrl, true); break;
                case GLFW_KEY_5: setStepParam(Clonotribe::PARAM_SEQUENCER_5_BUTTON, ctrl, true); break;
                case GLFW_KEY_6: setStepParam(Clonotribe::PARAM_SEQUENCER_6_BUTTON, ctrl, true); break;
                case GLFW_KEY_7: setStepParam(Clonotribe::PARAM_SEQUENCER_7_BUTTON, ctrl, true); break;
                case GLFW_KEY_8: setStepParam(Clonotribe::PARAM_SEQUENCER_8_BUTTON, ctrl, true); break;
                default: break;
            }
        } else if (e.action == GLFW_RELEASE) {
            switch (e.key) {
                case GLFW_KEY_F5: clonotribeModule->params[Clonotribe::PARAM_SYNTH_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_F6: clonotribeModule->params[Clonotribe::PARAM_BASSDRUM_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_F7: clonotribeModule->params[Clonotribe::PARAM_SNARE_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_F8: clonotribeModule->params[Clonotribe::PARAM_HIGHHAT_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_F9: clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_F10: clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_TAB: clonotribeModule->params[Clonotribe::PARAM_FLUX_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_LEFT_SHIFT: clonotribeModule->params[Clonotribe::PARAM_REC_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_SPACE: clonotribeModule->params[Clonotribe::PARAM_PLAY_BUTTON].setValue(ZERO); break;
                case GLFW_KEY_1: setStepParam(Clonotribe::PARAM_SEQUENCER_1_BUTTON, false, false); break;
                case GLFW_KEY_2: setStepParam(Clonotribe::PARAM_SEQUENCER_2_BUTTON, false, false); break;
                case GLFW_KEY_3: setStepParam(Clonotribe::PARAM_SEQUENCER_3_BUTTON, false, false); break;
                case GLFW_KEY_4: setStepParam(Clonotribe::PARAM_SEQUENCER_4_BUTTON, false, false); break;
                case GLFW_KEY_5: setStepParam(Clonotribe::PARAM_SEQUENCER_5_BUTTON, false, false); break;
                case GLFW_KEY_6: setStepParam(Clonotribe::PARAM_SEQUENCER_6_BUTTON, false, false); break;
                case GLFW_KEY_7: setStepParam(Clonotribe::PARAM_SEQUENCER_7_BUTTON, false, false); break;
                case GLFW_KEY_8: setStepParam(Clonotribe::PARAM_SEQUENCER_8_BUTTON, false, false); break;
                default: break;
            }
        }
    }

    void onHoverKey(const event::HoverKey& e) override {
        handleHoverKey(e);
        ModuleWidget::onHoverKey(e);
    }

    void appendContextMenu(rack::ui::Menu* menu) override {
        ModuleWidget::appendContextMenu(menu);
        Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
        if (clonotribeModule) {
            clonotribeModule->appendContextMenu(menu);
        }
    }
};


Model* modelClonotribe = createModel<Clonotribe, MainPanel>("Clonotribe");