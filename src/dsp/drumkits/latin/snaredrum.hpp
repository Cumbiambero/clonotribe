#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        ampEnv = 1.0f;
        toneEnv = 1.0f;
        noiseEnv = 1.0f;
        cracklEnv = 1.0f;
        tonePhase = 0.0f;
        highpassState = 0.0f;
        bandpassState1 = 0.0f;
        bandpassState2 = 0.0f;
        crackleFilter = 0.0f;
        triggered = true;
        sampleRate = 44100.0f; // TODO: Remove hard-coding
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Higher-pitched tone for Latin character
        const float toneFreq = 280.0f; // Brighter than typical snare
        
        // Generate tone component
        tonePhase += toneFreq * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase >= 2.0f * clonotribe::FastMath::PI) {
            tonePhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float tone = clonotribe::FastMath::fastSin(tonePhase) * toneEnv;
        
        // Generate bright, crispy noise
        float rawNoise = noise.process();
        
        // High-pass filter for brightness
        const float hpCutoff = 0.12f; // Remove low frequencies
        highpassState += (rawNoise - highpassState) * hpCutoff;
        float brightNoise = (rawNoise - highpassState) * noiseEnv;
        
        // Bandpass filter for mid-frequency crack
        const float bpCutoff = 0.2f; // Around 4.4kHz
        bandpassState1 += (brightNoise - bandpassState1) * bpCutoff;
        bandpassState2 += (bandpassState1 - bandpassState2) * bpCutoff;
        float crackNoise = (bandpassState1 - bandpassState2) * cracklEnv;
        
        // Additional crackle filter for texture
        const float crackleCutoff = 0.35f;
        crackleFilter += (crackNoise - crackleFilter) * crackleCutoff;
        float textureNoise = crackleFilter;
        
        // Mix components: less body, more crack and brightness
        float output = tone * 0.25f + brightNoise * 0.5f + crackNoise * 0.7f + textureNoise * 0.3f;
        
        // Latin-style envelopes: quick, crisp decay
        toneEnv *= 0.995f;     // Quick tone decay
        noiseEnv *= 0.988f;    // Medium noise decay
        cracklEnv *= 0.992f;   // Medium crackle decay
        ampEnv *= 0.990f;      // Overall amplitude
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        // Apply amplitude envelope and sharp saturation for snap
        output *= ampEnv;
        output = clonotribe::FastMath::fastTanh(output * 2.5f);
        
        return output * 1.3f;
    }
    
private:
    float ampEnv = 0.0f;
    float toneEnv = 0.0f;
    float noiseEnv = 0.0f;
    float cracklEnv = 0.0f;
    float tonePhase = 0.0f;
    float highpassState = 0.0f;
    float bandpassState1 = 0.0f;
    float bandpassState2 = 0.0f;
    float crackleFilter = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}