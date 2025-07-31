#pragma once
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        env = 1.0f;
        toneEnv = 1.0f;
        phase = 0.0f;
        triggered = true;
    }
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        float noiseVal = noise.process() * env;
        env *= 0.92f;
        float tone = 0.0f;
        if (toneEnv > 0.001f) {
            tone = 0.7f * toneEnv * clonotribe::FastMath::fastSin(phase * 2.0f * clonotribe::FastMath::PI);
            phase += 180.0f / 44100.0f;
            toneEnv *= 0.90f;
        }
        if (env < 0.001f && toneEnv < 0.001f) triggered = false;
        return (noiseVal * 0.5f + tone * 0.5f);
    }
private:
    float env = 0.0f;
    float toneEnv = 0.0f;
    float phase = 0.0f;
    bool triggered = false;
};

}
}
