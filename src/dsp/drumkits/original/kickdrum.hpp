#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        pitchEnv = ONE;
        ampEnv = ONE;
        subEnv = ONE;
        clickEnv = ONE;
        phase = ZERO;
        subPhase = ZERO;
        hpNoiseState = ZERO;
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    [[nodiscard]] float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) {
            return ZERO;
        }
        
        float invSampleRate = ONE / sampleRate;
        float accentGain = 0.75f + accent * HALF;
        
        float pitchMod = 110.0f * pitchEnv * pitchEnv;
        float freq = 58.0f + pitchMod;
        
        float analogDrift = ONE + noise.process() * 0.002f;
        phase += freq * analogDrift * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (phase >= TWO * clonotribe::FastMath::PI) {
            phase -= TWO * clonotribe::FastMath::PI;
        }
        
        subPhase += (freq * HALF) * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (subPhase >= TWO * clonotribe::FastMath::PI) {
            subPhase -= TWO * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * subEnv * 0.8f;
        
        float n = noise.process();
        
        hpNoiseState += (n - hpNoiseState) * HP_CUTOFF;
        float hpNoise = n - hpNoiseState;
        float click = (clickEnv > 0.85f ? (clickEnv - 0.85f) * 6.67f : ZERO) + hpNoise * 0.12f * clickEnv;
        float output = (mainSine * ampEnv + subSine + click * 0.25f);        
        float envDecay = 0.9983f + noise.process() * 0.0001f;
        
        pitchEnv *= 0.9988f;
        ampEnv *= envDecay;
        subEnv *= 0.9987f;
        clickEnv *= 0.988f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output = clonotribe::FastMath::fastTanh(output * 1.35f) * 0.9f;
        return output * 1.8f * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.25f;

    float pitchEnv = ZERO;
    float ampEnv = ZERO;
    float subEnv = ZERO;
    float clickEnv = ZERO;
    float phase = ZERO;
    float subPhase = ZERO;
    float hpNoiseState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}