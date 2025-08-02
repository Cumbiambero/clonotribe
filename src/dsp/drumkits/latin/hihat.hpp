#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace latin {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        env = 1.0f;
        shimmerEnv = 1.0f;
        phase1 = 0.0f;
        phase2 = 0.0f;
        phase3 = 0.0f;
        bandpass1State1 = 0.0f;
        bandpass1State2 = 0.0f;
        bandpass2State1 = 0.0f;
        bandpass2State2 = 0.0f;
        highpassState = 0.0f;
        triggered = true;
        sampleRate = 44100.0f; // TODO: Remove hard-coding
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // Shaker-like metallic tones (like maracas/tambourine)
        const float freq1 = 2500.0f;  // High metallic
        const float freq2 = 4200.0f;  // Higher metallic
        const float freq3 = 6800.0f;  // Very high shimmer
        
        // Generate metallic tones
        phase1 += freq1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase1 >= 2.0f * clonotribe::FastMath::PI) phase1 -= 2.0f * clonotribe::FastMath::PI;
        
        phase2 += freq2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase2 >= 2.0f * clonotribe::FastMath::PI) phase2 -= 2.0f * clonotribe::FastMath::PI;
        
        phase3 += freq3 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase3 >= 2.0f * clonotribe::FastMath::PI) phase3 -= 2.0f * clonotribe::FastMath::PI;
        
        // Create shimmery metallic sound
        float metallic1 = clonotribe::FastMath::fastSin(phase1) * env;
        float metallic2 = clonotribe::FastMath::fastSin(phase2) * env * 0.7f;
        float shimmer = clonotribe::FastMath::fastSin(phase3) * shimmerEnv * 0.5f;
        
        float metallicSum = metallic1 + metallic2 + shimmer;
        
        // High-frequency noise for texture
        float rawNoise = noise.process();
        
        // High-pass filter for brightness
        const float hpCutoff = 0.3f; // Very high cutoff
        highpassState += (rawNoise - highpassState) * hpCutoff;
        float brightNoise = (rawNoise - highpassState) * env;
        
        // Two bandpass filters for shaker character
        const float bp1Cutoff = 0.4f;  // Around 8.8kHz
        const float bp2Cutoff = 0.5f;  // Around 11kHz
        
        // First bandpass
        bandpass1State1 += (brightNoise - bandpass1State1) * bp1Cutoff;
        bandpass1State2 += (bandpass1State1 - bandpass1State2) * bp1Cutoff;
        float bp1Out = bandpass1State1 - bandpass1State2;
        
        // Second bandpass
        bandpass2State1 += (bp1Out - bandpass2State1) * bp2Cutoff;
        bandpass2State2 += (bandpass2State1 - bandpass2State2) * bp2Cutoff;
        float bp2Out = bandpass2State1 - bandpass2State2;
        
        // Mix metallic tones with filtered noise
        float output = metallicSum * 0.6f + bp2Out * 0.8f;
        
        // Shaker-style decay: medium length with shimmer tail
        env *= 0.9885f;        // Medium decay
        shimmerEnv *= 0.9935f; // Longer shimmer tail
        
        if (env < 0.001f && shimmerEnv < 0.001f) {
            triggered = false;
        }
        
        // Light saturation for character
        output = clonotribe::FastMath::fastTanh(output * 2.0f);
        
        return output * 0.9f;
    }
    
private:
    float env = 0.0f;
    float shimmerEnv = 0.0f;
    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float phase3 = 0.0f;
    float bandpass1State1 = 0.0f;
    float bandpass1State2 = 0.0f;
    float bandpass2State1 = 0.0f;
    float bandpass2State2 = 0.0f;
    float highpassState = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}
