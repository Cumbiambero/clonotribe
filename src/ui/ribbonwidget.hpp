#pragma once
#include <rack.hpp>

// Forward declaration
struct Clonotribe;

using namespace rack;

struct RibbonWidget : Widget {
    Clonotribe* module;
    bool dragging = false;
    Vec dragStartPos;
    
    RibbonWidget() {
        box.size = Vec(192.8, 26);
    }
    
    void setModule(Clonotribe* m) {
        module = m;
    }
    
    void onButton(const event::Button& e) override;
    void onDragMove(const event::DragMove& e) override;
    void updatePosition(Vec pos);
    void draw(const DrawArgs& args) override;
};
