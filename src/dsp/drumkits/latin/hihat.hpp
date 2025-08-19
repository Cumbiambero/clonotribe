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
        
        float metallic1 = clonotribe::FastMath::fastSin(phase1) * env;
        float metallic2 = clonotribe::FastMath::fastSin(phase2) * env * 0.7f;
        float shimmer = clonotribe::FastMath::fastSin(phase3) * shimmerEnv * 0.5f;
        float metallicSum = metallic1 + metallic2 + shimmer;
        float rawNoise = noise.process();
        
        highpassState += (rawNoise - highpassState) * HP_CUTOFF;
        float brightNoise = (rawNoise - highpassState) * env;
        
        bandpass1State1 += (brightNoise - bandpass1State1) * BP1_CUTOFF;
        bandpass1State2 += (bandpass1State1 - bandpass1State2) * BP1_CUTOFF;
        float bp1Out = bandpass1State1 - bandpass1State2;
        
        bandpass2State1 += (bp1Out - bandpass2State1) * BP2_CUTOFF;
        bandpass2State2 += (bandpass2State1 - bandpass2State2) * BP2_CUTOFF;
        float bp2Out = bandpass2State1 - bandpass2State2;
        
        float output = metallicSum * 0.55f + bp2Out * 0.85f;

        env *= 0.9890f;
        shimmerEnv *= 0.9940f;

        if (env < 0.001f && shimmerEnv < 0.001f) {
            triggered = false;
        }

        output = clonotribe::FastMath::fastTanh(output * 2.1f);        
        return output * 0.95f * accentGain;
    }
    
private:
    static constexpr float FREQ1 = 2300.0f;
    static constexpr float FREQ2 = 4000.0f;
    static constexpr float FREQ3 = 7200.0f;
    static constexpr float HP_CUTOFF = 0.27f;
    static constexpr float BP1_CUTOFF = 0.38f;
    static constexpr float BP2_CUTOFF = 0.48f;

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