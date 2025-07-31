#pragma once
#include "../../fastmath.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {


class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        env = 1.0f;
        phase = 0.0f;
        triggered = true;
    }
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        float f = 70.0f + 60.0f * env;
        float out = clonotribe::FastMath::fastSin(phase * 2.0f * clonotribe::FastMath::PI) * env;
        phase += f / 44100.0f;
        env *= 0.98f;
        if (env < 0.001f) triggered = false;
        return out * 0.7f;
    }
private:
    float phase = 0.0f;
    float env = 0.0f;
    bool triggered = false;
};

}
}
