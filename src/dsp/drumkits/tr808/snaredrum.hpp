#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        ampEnv = ONE;
        toneEnv = ONE;
        noiseEnv = ONE;
        tone1Phase = ZERO;
        tone2Phase = ZERO;
        bandpassState1 = ZERO;
        bandpassState2 = ZERO;
        highpassState = ZERO;
        triggered = true;
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    [[nodiscard]] float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) {
            return ZERO;
        }
        
        float invSampleRate = ONE / sampleRate;
        float accentGain = 0.8f + accent * 0.6f;
        
        tone1Phase += freq1 * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (tone1Phase >= TWO * clonotribe::FastMath::PI) {
            tone1Phase -= TWO * clonotribe::FastMath::PI;
        }
        
        tone2Phase += freq2 * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (tone2Phase >= TWO * clonotribe::FastMath::PI) {
            tone2Phase -= TWO * clonotribe::FastMath::PI;
        }
        
        float tone1 = clonotribe::FastMath::fastSin(tone1Phase) * toneEnv;
        float tone2 = clonotribe::FastMath::fastSin(tone2Phase) * toneEnv * 0.7f;
        float toneSum = tone1 + tone2;        
        float rawNoise = noise.process();
    
        bandpassState1 += (rawNoise - bandpassState1) * cutoff;
        bandpassState2 += (bandpassState1 - bandpassState2) * cutoff;
        float bandpassOut = bandpassState1 - bandpassState2;
    
        highpassState += (bandpassOut - highpassState) * hpCutoff;
        float filteredNoise = (bandpassOut - highpassState) * noiseEnv;        
        float output = toneSum * 0.35f + filteredNoise * 0.85f;

        toneEnv *= 0.993f;
        noiseEnv *= 0.9855f;
        ampEnv *= 0.9885f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output *= ampEnv;
        output = clonotribe::FastMath::fastTanh(output * 2.1f);
        return output * 1.5f * accentGain;
    }
    
private:
    const float cutoff = 0.17f;
    const float hpCutoff = 0.06f;
    const float freq1 = 330.0f;
    const float freq2 = 180.0f;

    float ampEnv = ZERO;
    float toneEnv = ZERO;
    float noiseEnv = ZERO;
    float tone1Phase = ZERO;
    float tone2Phase = ZERO;
    float bandpassState1 = ZERO;
    float bandpassState2 = ZERO;
    float highpassState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}