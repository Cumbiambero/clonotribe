#pragma once
#include "../../fastmath.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class KickDrum : public drumkits::KickDrum {
public:
    void reset() override {
        // Original monotribe kick: organic, warm, analog character
        pitchEnv = 1.0f;
        ampEnv = 1.0f;
        subEnv = 1.0f;
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
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Original monotribe character: warm, slightly detuned
        float pitchMod = 90.0f * pitchEnv * pitchEnv; // Wider pitch sweep
        float freq = 55.0f + pitchMod; // Lower base frequency
        
        // Main oscillator with slight analog drift
        float analogDrift = 1.0f + noise.process() * 0.002f; // Subtle drift
        phase += freq * analogDrift * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase >= 2.0f * clonotribe::FastMath::PI) {
            phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        // Sub-oscillator for depth (essential for analog character)
        subPhase += (freq * 0.5f) * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (subPhase >= 2.0f * clonotribe::FastMath::PI) {
            subPhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float mainSine = clonotribe::FastMath::fastSin(phase);
        float subSine = clonotribe::FastMath::fastSin(subPhase) * subEnv * 0.8f;
        
        // Organic click attack
        float click = clickEnv > 0.85f ? (clickEnv - 0.85f) * 6.67f : 0.0f;
        
        // Mix with analog-style proportions
        float output = (mainSine * ampEnv + subSine + click * 0.25f);
        
        // Organic envelope decay (slightly irregular)
        float envDecay = 0.9983f + noise.process() * 0.0001f; // Slight irregularity
        pitchEnv *= 0.9990f;  // Medium pitch decay
        ampEnv *= envDecay;   // Slightly irregular amplitude decay
        subEnv *= 0.9988f;    // Slower sub decay for warmth
        clickEnv *= 0.990f;   // Click decay
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        // Warm saturation characteristic of analog circuits
        output = clonotribe::FastMath::fastTanh(output * 1.3f) * 0.9f;
        return output * 1.8f; // Good level
    }
    
private:
    float pitchEnv = 0.0f;
    float ampEnv = 0.0f;
    float subEnv = 0.0f;
    float clickEnv = 0.0f;
    float phase = 0.0f;
    float subPhase = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}
