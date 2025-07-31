#pragma once
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {


class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        env = 1.0f;
        triggered = true;
    }
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        float noiseVal = noise.process() * env;
        float tone = env * 0.3f;
        env *= 0.91f;
        if (env < 0.001f) triggered = false;
        return (noiseVal * 0.6f + tone) * 0.7f;
    }
private:
    float env = 0.0f;
    bool triggered = false;
};

}
}