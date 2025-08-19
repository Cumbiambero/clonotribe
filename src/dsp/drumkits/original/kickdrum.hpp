#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        subEnv = 1.0f;
        clickEnv = 1.0f;
        phase = 0.0f;
        subPhase = 0.0f;
        hpNoiseState = 0.0f;
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) {
            return 0.0f;
        }
        
        float invSampleRate = 1.0f / sampleRate;
        float accentGain = 0.75f + accent * 0.5f;
        
        float pitchMod = 110.0f * pitchEnv * pitchEnv;
        float freq = 58.0f + pitchMod;
        
        float analogDrift = 1.0f + noise.process() * 0.002f;
        phase += freq * analogDrift * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase >= 2.0f * clonotribe::FastMath::PI) {
            phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        subPhase += (freq * 0.5f) * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (subPhase >= 2.0f * clonotribe::FastMath::PI) {
            subPhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * subEnv * 0.8f;
        
        float n = noise.process();
        
        hpNoiseState += (n - hpNoiseState) * HP_CUTOFF;
        float hpNoise = n - hpNoiseState;
        float click = (clickEnv > 0.85f ? (clickEnv - 0.85f) * 6.67f : 0.0f) + hpNoise * 0.12f * clickEnv;
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

    float pitchEnv = 0.0f;
    float ampEnv = 0.0f;
    float subEnv = 0.0f;
    float clickEnv = 0.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;
    float hpNoiseState = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}