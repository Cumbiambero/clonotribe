#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        ampEnv = 1.0f;
        toneEnv = 1.0f;
        noiseEnv = 1.0f;
        tone1Phase = 0.0f;
        tone2Phase = 0.0f;
        bandpassState1 = 0.0f;
        bandpassState2 = 0.0f;
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
        
        tone1Phase += freq1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tone1Phase >= 2.0f * clonotribe::FastMath::PI) {
            tone1Phase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        tone2Phase += freq2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tone2Phase >= 2.0f * clonotribe::FastMath::PI) {
            tone2Phase -= 2.0f * clonotribe::FastMath::PI;
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

    float ampEnv = 0.0f;
    float toneEnv = 0.0f;
    float noiseEnv = 0.0f;
    float tone1Phase = 0.0f;
    float tone2Phase = 0.0f;
    float bandpassState1 = 0.0f;
    float bandpassState2 = 0.0f;
    float highpassState = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}