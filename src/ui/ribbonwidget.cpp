#include "ribbonwidget.hpp"
#include "../clonotribe.hpp"

void RibbonWidget::onButton(const event::Button& e) {
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        dragging = true;
        dragStartPos = e.pos;
        updatePosition(e.pos);
        e.consume(this);
    } else if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        dragging = false;
        if (module) {
            module->ribbon.setTouching(false);
        }
        e.consume(this);
    }
}

void RibbonWidget::onDragMove(const event::DragMove& e) {
    if (dragging) {
        dragStartPos = dragStartPos.plus(e.mouseDelta);
        updatePosition(dragStartPos);
    }
}

void RibbonWidget::updatePosition(Vec pos) {
    if (!module) return;
    
    float margin = 6.0f; // Same as dot radius
    float effectiveWidth = box.size.x - 2 * margin;
    float adjustedX = clamp(pos.x - margin, 0.f, effectiveWidth);
    
    float position = adjustedX / effectiveWidth;
    module->ribbon.setPosition(position);
    module->ribbon.setTouching(true);
}

void RibbonWidget::draw(const DrawArgs& args) {
    if (module && module->ribbon.touching) {
        float margin = 6.0f;
        float effectiveWidth = box.size.x - 2 * margin;
        float pos = margin + (module->ribbon.position * effectiveWidth);
        
        nvgBeginPath(args.vg);
        nvgCircle(args.vg, pos, box.size.y * 0.5f, 6);
        nvgFillColor(args.vg, nvgRGB(255, 100, 100));
        nvgFill(args.vg);
    }
    
    Widget::draw(args);
}
