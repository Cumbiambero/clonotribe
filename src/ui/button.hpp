#pragma once
#include <rack.hpp>

namespace clonotribe {

struct TransparentButton final : rack::app::ParamWidget {
    bool pressed = false;

    TransparentButton() noexcept {
        box.size = rack::Vec(24, 18);
    }

    void draw(const DrawArgs& args) override {
        if (getParamQuantity() && getParamQuantity()->getValue() > 0.5f) {
            nvgBeginPath(args.vg);
            nvgRect(args.vg, 0, 0, box.size.x, box.size.y);
            nvgStrokeColor(args.vg, nvgRGBA(255, 255, 255, 50));
            nvgStrokeWidth(args.vg, ONE);
            nvgStroke(args.vg);
        }
    }

    void onButton(const rack::event::Button& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            if (e.action == GLFW_PRESS) {
                pressed = true;
                if (auto* q = getParamQuantity()) {
                    int mods = APP->window->getMods();
                    bool ctrl = (mods & RACK_MOD_CTRL) != 0;
                    q->setValue(ctrl ? 0.9f : ONE);
                }
            } else if (e.action == GLFW_RELEASE) {
                if (getParamQuantity()) {
                    getParamQuantity()->setValue(ZERO);
                }
            }
        }
        rack::app::ParamWidget::onButton(e);
    }

    void onDragStart(const rack::event::DragStart& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
            if (auto* q = getParamQuantity()) {
                int mods = APP->window->getMods();
                bool ctrl = (mods & RACK_MOD_CTRL) != 0;
                q->setValue(ctrl ? 0.9f : ONE);
            }
        }
        rack::app::ParamWidget::onDragStart(e);
    }
    
    void onDragEnd(const rack::event::DragEnd& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && pressed) {
            pressed = false;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(ZERO);
            }
        }
        ParamWidget::onDragEnd(e);
    }
};

}