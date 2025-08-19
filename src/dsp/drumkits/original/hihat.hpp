#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
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
    
    [[nodiscard]] float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) {
            return 0.0f;
        }
        
        float invSampleRate = 1.0f / sampleRate;
        float accentGain = 0.7f + accent * 0.6f;
        
        phase1 += FREQ1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase1 >= 2.0f * clonotribe::FastMath::PI) {
            phase1 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        phase2 += FREQ2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase2 >= 2.0f * clonotribe::FastMath::PI) {
            phase2 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        phase3 += FREQ3 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase3 >= 2.0f * clonotribe::FastMath::PI) {
            phase3 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        phase4 += FREQ4 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (phase4 >= 2.0f * clonotribe::FastMath::PI) {
            phase4 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float metallic1 = clonotribe::FastMath::fastSin(phase1) * env;
        float metallic2 = clonotribe::FastMath::fastSin(phase2) * env * 0.8f;
        float metallic3 = clonotribe::FastMath::fastSin(phase3) * shimmerEnv * 0.6f;
        float metallic4 = clonotribe::FastMath::fastSin(phase4) * shimmerEnv * 0.4f;
        float metallicSum = metallic1 + metallic2 + metallic3 + metallic4;
        float rawNoise = noise.process();
        
        highpass += (rawNoise - highpass) * HP_CUTOFF;
        float brightNoise = (rawNoise - highpass) * env;
        
        bandpass1 += (brightNoise - bandpass1) * BP_CUTOFF;
        bandpass2 += (bandpass1 - bandpass2) * BP_CUTOFF;
        float filteredNoise = bandpass1 - bandpass2;        
        float output = metallicSum * 0.55f + filteredNoise * 0.75f;

        env *= 0.9895f;
        shimmerEnv *= 0.9925f;

        if (env < 0.001f && shimmerEnv < 0.001f) {
            triggered = false;
        }

        output = clonotribe::FastMath::fastTanh(output * 1.6f);
        return output * 1.0f * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.2f;
    static constexpr float BP_CUTOFF = 0.32f;
    static constexpr float FREQ1 = 7200.0f;
    static constexpr float FREQ2 = 8800.0f;
    static constexpr float FREQ3 = 11200.0f;
    static constexpr float FREQ4 = 13600.0f;

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