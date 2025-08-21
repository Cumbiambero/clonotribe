#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        pitchEnv = ONE;
        ampEnv = ONE;
        clickEnv = ONE;
        phase = ZERO;
        lowPhase = ZERO;
        hpState = ZERO;
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
        float accentGain = 0.8f + accent * 0.6f;        
        float pitchMod = 45.0f * pitchEnv * pitchEnv;
        float freq = 92.0f + pitchMod;
        
        phase += freq * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (phase >= TWO * clonotribe::FastMath::PI) {
            phase -= TWO * clonotribe::FastMath::PI;
        }
        
        lowPhase += (freq * 0.6f) * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (lowPhase >= TWO * clonotribe::FastMath::PI) {
            lowPhase -= TWO * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float lowSine = clonotribe::FastMath::fastSin(lowPhase) * 0.4f;
        
        float n = noise.process();
        
        hpState += (n - hpState) * HP_CUTOFF;
        float hpNoise = n - hpState;
        float click = (clickEnv > 0.7f ? (clickEnv - 0.7f) * 3.33f : ZERO) + hpNoise * 0.1f * clickEnv;        
        float output = (mainSine + lowSine + click * 0.4f) * ampEnv;
        
        pitchEnv *= 0.9986f;
        ampEnv *= 0.9978f;
        clickEnv *= 0.987f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output = clonotribe::FastMath::fastTanh(output * 1.9f);
        return output * 1.45f * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.28f;

    float pitchEnv = ZERO;
    float ampEnv = ZERO;
    float clickEnv = ZERO;
    float phase = ZERO;
    float lowPhase = ZERO;
    float hpState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}