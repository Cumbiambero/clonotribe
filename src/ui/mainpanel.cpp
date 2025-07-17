#include "../clonotribe.hpp"

struct MainPanel : ModuleWidget {
    TransparentButton* createButton(Vec pos, Vec size, Module* module, int paramId) {
        auto* button = createParam<TransparentButton>(pos, module, paramId);
        button->box.size = size;
        return button;
    }
    
    MainPanel(Clonotribe* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));

        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0, 42.0)), module, Clonotribe::PARAM_VCO_WAVEFORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(35.0, 62.5)), module, Clonotribe::PARAM_RIBBON_RANGE_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(77.0, 62.5)), module, Clonotribe::PARAM_ENVELOPE_FORM_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(94.0, 62.5)), module, Clonotribe::PARAM_LFO_TARGET_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(109.0, 62.5)), module, Clonotribe::PARAM_LFO_MODE_SWITCH));
        addParam(createParamCentered<CKSSThree>(mm2px(Vec(123.0, 62.5)), module, Clonotribe::PARAM_LFO_WAVEFORM_SWITCH));

        addParam(createParamCentered<OctaveKnob>(mm2px(Vec(15.0, 42.0)), module, Clonotribe::PARAM_VCO_OCTAVE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5, 42.0)), module, Clonotribe::PARAM_VCF_CUTOFF_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(77.0, 42.0)), module, Clonotribe::PARAM_VCA_LEVEL_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(98.0, 42.0)), module, Clonotribe::PARAM_LFO_RATE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(119.0, 42.0)), module, Clonotribe::PARAM_LFO_INTERVAL_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0, 42.0)), module, Clonotribe::PARAM_RHYTM_VOLUME_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(15.0, 62.5)), module, Clonotribe::PARAM_NOISE_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(56.5, 62.5)), module, Clonotribe::PARAM_VCF_PEAK_KNOB));
        addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(140.0, 62.5)), module, Clonotribe::PARAM_SEQUENCER_TEMPO_KNOB));

        addParam(createButton(mm2px(Vec(30.25, 103.25)), Vec(51.0, 18.0), module, Clonotribe::PARAM_PLAY_BUTTON));
        addParam(createButton(mm2px(Vec(11.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SYNTH_BUTTON));
        addParam(createButton(mm2px(Vec(20.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_BASSDRUM_BUTTON));
        addParam(createButton(mm2px(Vec(30.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SNARE_BUTTON));
        addParam(createButton(mm2px(Vec(39.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_HIGHHAT_BUTTON));
        addParam(createButton(mm2px(Vec(11.0, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_FLUX_BUTTON));
        addParam(createButton(mm2px(Vec(20.5, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_REC_BUTTON));
        addParam(createButton(mm2px(Vec(68.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_1_BUTTON));
        addParam(createButton(mm2px(Vec(78.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_2_BUTTON));
        addParam(createButton(mm2px(Vec(88.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_3_BUTTON));
        addParam(createButton(mm2px(Vec(97.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_4_BUTTON));
        addParam(createButton(mm2px(Vec(107.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_5_BUTTON));
        addParam(createButton(mm2px(Vec(117.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_6_BUTTON));
        addParam(createButton(mm2px(Vec(126.5, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_7_BUTTON));
        addParam(createButton(mm2px(Vec(136.0, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_SEQUENCER_8_BUTTON));
        addParam(createButton(mm2px(Vec(49.25, 84.5)), Vec(24, 18), module, Clonotribe::PARAM_ACTIVE_STEP_BUTTON));
        addParam(createButton(mm2px(Vec(49.25, 103.25)), Vec(24, 18), module, Clonotribe::PARAM_GATE_TIME_BUTTON));
       
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

        addChild(new Ribbon(module));
    }

    void onHoverKey(const event::HoverKey& e) override {
        if (e.action == GLFW_PRESS || e.action == GLFW_REPEAT) {
            Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
            if (!clonotribeModule) return;
            switch (e.key) {
                case GLFW_KEY_F7:
                    if (clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_F8:
                    if (clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(1.f);
                    }          
                    break;
                case GLFW_KEY_1:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_2:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_3:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_4:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].setValue(1.f);
                    break;
                case GLFW_KEY_5:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_6:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].setValue(1.f);
                    }     
                    break;
                case GLFW_KEY_7:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].setValue(1.f);
                    }
                    break;
                case GLFW_KEY_8:
                    if (clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].getValue() < 0.5f) {
                        clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].setValue(1.f);
                    }
                }
            }
        } else if (e.action == GLFW_RELEASE) {
            Clonotribe* clonotribeModule = dynamic_cast<Clonotribe*>(module);
            if (!clonotribeModule) return;
            switch (e.key) {
                case GLFW_KEY_F7:
                    clonotribeModule->params[Clonotribe::PARAM_ACTIVE_STEP_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_F8:
                    clonotribeModule->params[Clonotribe::PARAM_GATE_TIME_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_1:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_1_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_2:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_2_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_3:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_3_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_4:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_4_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_5:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_5_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_6:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_6_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_7:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_7_BUTTON].setValue(0.f);
                    break;
                case GLFW_KEY_8:
                    clonotribeModule->params[Clonotribe::PARAM_SEQUENCER_8_BUTTON].setValue(0.f);
            }
            e.consume(this); 
        }
        ModuleWidget::onHoverKey(e);
    }
};


Model* modelClonotribe = createModel<Clonotribe, MainPanel>("Clonotribe");