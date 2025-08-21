#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        pitchEnv = ONE;
        ampEnv = ONE;
        clickEnv = ONE;
        phase = ZERO;
        subPhase = ZERO;
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
        float pitchMod = 60.0f * pitchEnv * pitchEnv * pitchEnv;
        float freq = 60.0f + pitchMod;
        
        phase += freq * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (phase >= TWO * clonotribe::FastMath::PI) {
            phase -= TWO * clonotribe::FastMath::PI;
        }
        
        subPhase += (freq * 0.5f) * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (subPhase >= TWO * clonotribe::FastMath::PI) {
            subPhase -= TWO * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * 0.6f;        
        float n = noise.process();
        
        hpState += (n - hpState) * HP_CUTOFF;
        float hpNoise = n - hpState;
        float click = (clickEnv > 0.8f ? (clickEnv - 0.8f) * 5.0f : ZERO) + hpNoise * 0.08f * clickEnv;
        float output = (mainSine + subSine + click * 0.3f) * ampEnv * ampEnv;
        
        pitchEnv *= 0.9992f;
        ampEnv *= 0.9986f;
        clickEnv *= 0.9915f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output = clonotribe::FastMath::fastTanh(output * 1.55f);
        return output * TWO * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.25f;

    float pitchEnv = ZERO;
    float ampEnv = ZERO;
    float clickEnv = ZERO;
    float phase = ZERO;
    float subPhase = ZERO;
    float hpState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}