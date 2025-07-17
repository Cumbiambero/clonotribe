#pragma once
#include <rack.hpp>

struct Clonotribe;

using namespace rack;

struct Ribbon : Widget {
    Clonotribe* module;
    bool dragging = false;
    Vec dragStartPos;
    
    Ribbon(Clonotribe* m) : module(m) {
        box.size = Vec(192.8, 26);
        box.pos = mm2px(Vec(73.8, 99.2)); 
    }
        
    void onButton(const event::Button& e) override;
    void onDragMove(const event::DragMove& e) override;
    void updatePosition(Vec pos);
    void draw(const DrawArgs& args) override;
};
