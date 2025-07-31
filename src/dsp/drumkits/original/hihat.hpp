#pragma once
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        env = 1.0f;
        phase1 = phase2 = phase3 = 0.0f;
        triggered = true;
    }
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        float metallic = 0.0f;
        metallic += clonotribe::FastMath::fastSin(phase1 * 2.0f * clonotribe::FastMath::PI);
        metallic += clonotribe::FastMath::fastSin(phase2 * 2.0f * clonotribe::FastMath::PI);
        metallic += clonotribe::FastMath::fastSin(phase3 * 2.0f * clonotribe::FastMath::PI);
        phase1 += 8000.0f / 44100.0f;
        phase2 += 9000.0f / 44100.0f;
        phase3 += 10000.0f / 44100.0f;
        float noiseVal = (noise.process() > 0.0f ? 1.0f : -1.0f);
        float out = (metallic * 0.5f + noiseVal * 0.5f) * env;
        env *= 0.93f;
        if (env < 0.001f) triggered = false;
        return out * 0.3f;
    }
private:
    float env = 0.0f;
    float phase1 = 0.0f, phase2 = 0.0f, phase3 = 0.0f;
    bool triggered = false;
};

}
}
