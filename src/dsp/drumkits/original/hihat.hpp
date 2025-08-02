#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        // Original monotribe hi-hat: metallic but not overly harsh
        env = 1.0f;
        shimmerEnv = 1.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        phase3 = 0.0f;
        phase4 = 0.0f;
        bandpass1 = 0.0f;
        bandpass2 = 0.0f;
        highpass = 0.0f;
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Original monotribe hi-hat: 4 sine waves for metallic character
        const float freq1 = 7200.0f;  // Primary metallic
        const float freq2 = 8800.0f;  // Secondary metallic
        const float freq3 = 11200.0f; // High sparkle
        const float freq4 = 13600.0f; // Very high shimmer
        
        // Generate metallic tones
        phase1 += freq1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase1 >= 2.0f * clonotribe::FastMath::PI) phase1 -= 2.0f * clonotribe::FastMath::PI;
        
        phase2 += freq2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase2 >= 2.0f * clonotribe::FastMath::PI) phase2 -= 2.0f * clonotribe::FastMath::PI;
        
        phase3 += freq3 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase3 >= 2.0f * clonotribe::FastMath::PI) phase3 -= 2.0f * clonotribe::FastMath::PI;
        
        phase4 += freq4 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase4 >= 2.0f * clonotribe::FastMath::PI) phase4 -= 2.0f * clonotribe::FastMath::PI;
        
        float metallic1 = clonotribe::FastMath::fastSin(phase1) * env;
        float metallic2 = clonotribe::FastMath::fastSin(phase2) * env * 0.8f;
        float metallic3 = clonotribe::FastMath::fastSin(phase3) * shimmerEnv * 0.6f;
        float metallic4 = clonotribe::FastMath::fastSin(phase4) * shimmerEnv * 0.4f;
        
        float metallicSum = metallic1 + metallic2 + metallic3 + metallic4;
        
        // Generate filtered noise
        float rawNoise = noise.process();
        
        // High-pass filter for brightness
        const float hpCutoff = 0.2f;
        highpass += (rawNoise - highpass) * hpCutoff;
        float brightNoise = (rawNoise - highpass) * env;
        
        // Bandpass filter for classic hi-hat character
        const float bpCutoff = 0.3f;
        bandpass1 += (brightNoise - bandpass1) * bpCutoff;
        bandpass2 += (bandpass1 - bandpass2) * bpCutoff;
        float filteredNoise = bandpass1 - bandpass2;
        
        // Mix metallic tones with filtered noise (original monotribe balance)
        float output = metallicSum * 0.6f + filteredNoise * 0.7f;
        
        // Original-style decay
        env *= 0.9890f;        // Medium decay
        shimmerEnv *= 0.9920f; // Longer shimmer tail
        
        if (env < 0.001f && shimmerEnv < 0.001f) {
            triggered = false;
        }
        
        // Light saturation for character
        output = clonotribe::FastMath::fastTanh(output * 1.5f);
        
        return output * 1.0f;
    }
    
private:
    float env = 0.0f;
    float shimmerEnv = 0.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;
    float phase4 = 0.0f;
    float bandpass1 = 0.0f;
    float bandpass2 = 0.0f;
    float highpass = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}