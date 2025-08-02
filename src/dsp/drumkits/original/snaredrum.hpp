#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        // Original monotribe snare: balanced tone and noise
        ampEnv = 1.0f;
        toneEnv = 1.0f;
        noiseEnv = 1.0f;
        buzzEnv = 1.0f;
        tonePhase1 = 0.0f;
        tonePhase2 = 0.0f;
        noiseFilter1 = 0.0f;
        noiseFilter2 = 0.0f;
        bodyFilter = 0.0f;
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Original monotribe snare frequencies (fundamental + harmonic)
        const float freq1 = 220.0f;  // Fundamental tone
        const float freq2 = 350.0f;  // Harmonic for body
        
        // Generate dual-tone body
        tonePhase1 += freq1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase1 >= 2.0f * clonotribe::FastMath::PI) tonePhase1 -= 2.0f * clonotribe::FastMath::PI;
        
        tonePhase2 += freq2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase2 >= 2.0f * clonotribe::FastMath::PI) tonePhase2 -= 2.0f * clonotribe::FastMath::PI;
        
        float tone1 = clonotribe::FastMath::fastSin(tonePhase1) * toneEnv;
        float tone2 = clonotribe::FastMath::fastSin(tonePhase2) * toneEnv * 0.6f;
        
        // Body filter for warmth
        float toneSum = tone1 + tone2;
        bodyFilter += (toneSum - bodyFilter) * 0.8f; // Smooth body tone
        
        // Generate noise component
        float rawNoise = noise.process();
        
        // Two-stage noise filtering for classic snare buzz
        const float cutoff1 = 0.3f; // Initial filtering
        const float cutoff2 = 0.15f; // Final shaping
        
        noiseFilter1 += (rawNoise - noiseFilter1) * cutoff1;
        noiseFilter2 += (noiseFilter1 - noiseFilter2) * cutoff2;
        float buzzNoise = (noiseFilter1 - noiseFilter2) * buzzEnv;
        
        // Mix components with classic snare balance
        float bodyTone = bodyFilter * 0.5f;
        float snareNoise = buzzNoise * 0.7f;
        float output = bodyTone + snareNoise;
        
        // Original-style envelope decay
        toneEnv *= 0.994f;   // Medium tone decay
        buzzEnv *= 0.986f;   // Slower buzz decay for classic tail
        noiseEnv *= 0.988f;  // Noise envelope
        ampEnv *= 0.990f;    // Overall amplitude
        
        if (ampEnv < 0.001f && buzzEnv < 0.001f) {
            triggered = false;
        }
        
        // Apply amplitude envelope and gentle saturation
        output *= ampEnv;
        output = clonotribe::FastMath::fastTanh(output * 1.6f);
        
        return output * 1.4f;
    }
    
private:
    float ampEnv = 0.0f;
    float toneEnv = 0.0f;
    float noiseEnv = 0.0f;
    float buzzEnv = 0.0f;
    float tonePhase1 = 0.0f;
    float tonePhase2 = 0.0f;
    float noiseFilter1 = 0.0f;
    float noiseFilter2 = 0.0f;
    float bodyFilter = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}