#pragma once
#include <rack.hpp>
#include "../dsp/fastmath.hpp"

namespace clonotribe {

struct OctaveKnob final : rack::RoundBlackKnob {
    OctaveKnob() noexcept {
        minAngle = -0.4f * FastMath::PI;
        maxAngle = 0.4f * FastMath::PI;
        snap = true;
    }
};
}
