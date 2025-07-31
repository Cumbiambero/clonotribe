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
            nvgStrokeWidth(args.vg, 1.0f);
            nvgStroke(args.vg);
        }
    }

    void onButton(const rack::event::Button& e) override {
        if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(1.0f);
            }
            e.consume(this);
        }
        rack::app::ParamWidget::onButton(e);
    }

    void onDragStart(const rack::event::DragStart& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(1.0f);
            }
        }
        rack::app::ParamWidget::onDragStart(e);
    }
    
    void onDragEnd(const rack::event::DragEnd& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && pressed) {
            pressed = false;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(0.0f);
            }
        }
        ParamWidget::onDragEnd(e);
    }
};
}