#pragma once
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {


class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        env = 1.0f;
        triggered = true;
    }
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        float out = 0.0f;
        for (int i = 0; i < 3; ++i) out += (noise.process() > 0.0f ? 1.0f : -1.0f);
        out = (out / 3.0f) * env;
        env *= 0.92f;
        if (env < 0.001f) triggered = false;
        return out * 0.5f;
    }
private:
    float env = 0.0f;
    bool triggered = false;
};

}
}
