#pragma once
#include <rack.hpp>

using namespace rack;
struct TransparentMomentaryButton : app::ParamWidget {
    TransparentMomentaryButton() {
        box.size = Vec(24, 18);
    }
    
    void draw(const DrawArgs& args) override {
        if (getParamQuantity() && getParamQuantity()->getValue() > 0.5f) {
            nvgBeginPath(args.vg);
            nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
            nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 50));
            nvgStrokeWidth(args.vg, 1.0f);
            nvgStroke(args.vg);
        }
    }
    
    void onButton(const event::Button& e) override {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (getParamQuantity()) {
                getParamQuantity()->setValue(1.f);
            }
            e.consume(this);
        } else if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (getParamQuantity()) {
                getParamQuantity()->setValue(0.f);
            }
            e.consume(this);
        }
        ParamWidget::onButton(e);
    }
};