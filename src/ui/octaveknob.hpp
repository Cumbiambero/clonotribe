#pragma once
#include <rack.hpp>

using namespace rack;

struct OctaveKnob : RoundBlackKnob {
    OctaveKnob() {
        minAngle = -0.4f * M_PI;
        maxAngle = 0.4f * M_PI;
        snap = true;
    }
};
