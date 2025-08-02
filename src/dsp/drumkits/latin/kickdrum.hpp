#pragma once
#include "../../fastmath.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        // Latin kick: tight, punchy, higher-pitched than 808
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        clickEnv = 1.0f;
        phase = 0.0f;
        lowPhase = 0.0f;
        triggered = true;
        sampleRate = 44100.0f; // TODO: Remove hard-coding
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Latin kick: higher fundamental, tighter envelope
        float pitchMod = 40.0f * pitchEnv * pitchEnv; // Less dramatic pitch sweep
        float freq = 85.0f + pitchMod; // Higher base frequency
        
        // Main oscillator
        phase += freq * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase >= 2.0f * clonotribe::FastMath::PI) {
            phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        // Low-frequency component for body
        lowPhase += (freq * 0.6f) * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (lowPhase >= 2.0f * clonotribe::FastMath::PI) {
            lowPhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float lowSine = clonotribe::FastMath::fastSin(lowPhase) * 0.4f;
        
        // Sharper click for cumbia character
        float click = clickEnv > 0.7f ? (clickEnv - 0.7f) * 3.33f : 0.0f;
        
        // Mix with more prominence on main tone
        float output = (mainSine + lowSine + click * 0.4f) * ampEnv;
        
        // Faster, tighter decay for Latin style
        pitchEnv *= 0.9988f;  // Faster pitch decay
        ampEnv *= 0.9975f;    // Faster amplitude decay
        clickEnv *= 0.988f;   // Sharp click decay
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        // Light saturation for punch
        output = clonotribe::FastMath::fastTanh(output * 1.8f);
        return output * 1.4f; // Punchy output
    }
    
private:
    float pitchEnv = 0.0f;
    float ampEnv = 0.0f;
    float clickEnv = 0.0f;
    float phase = 0.0f;
    float lowPhase = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}
