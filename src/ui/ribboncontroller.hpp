#pragma once
#include "../constants.hpp"
#include <rack.hpp>

struct Clonotribe;

namespace clonotribe {

struct RibbonController final : rack::Widget {
    Clonotribe* module = nullptr;
    bool dragging = false;
    rack::Vec dragStartPos;

    explicit RibbonController(Clonotribe* m) noexcept : module(m) {
        box.size = rack::Vec(192.8f, 26.0f);
        box.pos = rack::window::mm2px(rack::Vec(73.8f, 99.2f));
    }

    void onButton(const rack::event::Button& e) override;
    void onDragMove(const rack::event::DragMove& e) override;
    void updatePosition(rack::Vec pos) noexcept;
    void draw(const DrawArgs& args) override;

private:
    float position = ZERO;
};
}