#pragma once
#include "../../fastmath.hpp"
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
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        // TR-808 kick synthesis with proper envelopes
        const float invSampleRate = 1.0f / sampleRate;
        
        // Authentic TR-808 pitch envelope: fast exponential decay
        float pitchMod = 60.0f * pitchEnv * pitchEnv * pitchEnv; // 60Hz base + pitch mod
        float freq = 60.0f + pitchMod;
        
        // Main oscillator
        phase += freq * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase >= 2.0f * clonotribe::FastMath::PI) {
            phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        // Sub-harmonic for authentic 808 character
        subPhase += (freq * 0.5f) * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (subPhase >= 2.0f * clonotribe::FastMath::PI) {
            subPhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * 0.6f;
        
        // Attack click (essential for 808 character)
        float click = clickEnv > 0.8f ? (clickEnv - 0.8f) * 5.0f : 0.0f;
        
        // Mix oscillators
        float output = (mainSine + subSine + click * 0.3f) * ampEnv * ampEnv;
        
        // Authentic TR-808 envelope decay rates
        pitchEnv *= 0.9992f;  // Very fast pitch decay
        ampEnv *= 0.9985f;    // Slower amplitude decay
        clickEnv *= 0.992f;   // Medium click decay
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        // Soft saturation for analog character
        output = clonotribe::FastMath::fastTanh(output * 1.5f);
        return output * 2.0f; // Final gain
    }
    
private:
    float pitchEnv = 0.0f;
    float ampEnv = 0.0f;
    float clickEnv = 0.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}