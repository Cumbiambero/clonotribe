#include "ribboncontroller.hpp"
#include "../clonotribe.hpp"

void RibbonController::onButton(const event::Button& e) {
    if (e.action == GLFW_PRESS && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        dragging = true;
        dragStartPos = e.pos;
        updatePosition(e.pos);
        e.consume(this);
    } else if (e.action == GLFW_RELEASE && e.button == GLFW_MOUSE_BUTTON_LEFT) {
        dragging = false;
        if (module) {
            module->ribbon.touching = false;
        }
        e.consume(this);
    }
}

void RibbonController::onDragMove(const event::DragMove& e) {
    if (dragging) {
        dragStartPos = dragStartPos.plus(e.mouseDelta);
        updatePosition(dragStartPos);
    }
}

void RibbonController::updatePosition(Vec pos) noexcept {
    if (!module) return;
    
    float margin = 6.0f;
    float effectiveWidth = box.size.x - 2 * margin;
    float adjustedX = clamp(pos.x - margin, 0.0f, effectiveWidth);
    
    float position = adjustedX / effectiveWidth;
    module->ribbon.setPosition(position);
    module->ribbon.touching = true;
}

void RibbonController::draw(const DrawArgs& args) {
    if (module && module->ribbon.touching) {
        float margin = 6.0f;
        float effectiveWidth = box.size.x - 2 * margin;
        float pos = margin + (module->ribbon.getPosition() * effectiveWidth);

        nvgBeginPath(args.vg);
        nvgCircle(args.vg, pos, box.size.y * 0.5f, 6);
        nvgFillColor(args.vg, nvgRGBA(255, 100, 100, 200));
        nvgFill(args.vg);
    }
    
    Widget::draw(args);
}
