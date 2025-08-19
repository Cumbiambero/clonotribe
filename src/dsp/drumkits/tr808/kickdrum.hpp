#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        clickEnv = 1.0f;
        phase = 0.0f;
        subPhase = 0.0f;
        hpState = 0.0f;
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
        float accentGain = 0.8f + accent * 0.6f;        
        float pitchMod = 60.0f * pitchEnv * pitchEnv * pitchEnv;
        float freq = 60.0f + pitchMod;
        
        phase += freq * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase >= 2.0f * clonotribe::FastMath::PI) {
            phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        subPhase += (freq * 0.5f) * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (subPhase >= 2.0f * clonotribe::FastMath::PI) {
            subPhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * 0.6f;        
        float n = noise.process();
        
        hpState += (n - hpState) * HP_CUTOFF;
        float hpNoise = n - hpState;
        float click = (clickEnv > 0.8f ? (clickEnv - 0.8f) * 5.0f : 0.0f) + hpNoise * 0.08f * clickEnv;
        float output = (mainSine + subSine + click * 0.3f) * ampEnv * ampEnv;
        
        pitchEnv *= 0.9992f;
        ampEnv *= 0.9986f;
        clickEnv *= 0.9915f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output = clonotribe::FastMath::fastTanh(output * 1.55f);
        return output * 2.0f * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.25f;

    float pitchEnv = 0.0f;
    float ampEnv = 0.0f;
    float clickEnv = 0.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;
    float hpState = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}