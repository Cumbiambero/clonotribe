#pragma once
#include <rack.hpp>

using namespace rack;
struct TransparentButton : app::ParamWidget {
    bool pressed = false;
    
    TransparentButton() {
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
            pressed = true;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(1.f);
            }
            e.consume(this);
        }
        ParamWidget::onButton(e);
    }
    
    void onDragStart(const event::DragStart& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
            pressed = true;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(1.f);
            }
        }
        ParamWidget::onDragStart(e);
    }
    
    void onDragEnd(const event::DragEnd& e) override {
        if (e.button == GLFW_MOUSE_BUTTON_LEFT && pressed) {
            pressed = false;
            if (getParamQuantity()) {
                getParamQuantity()->setValue(0.f);
            }
        }
        ParamWidget::onDragEnd(e);
    }
};