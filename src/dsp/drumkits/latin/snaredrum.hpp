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
    }
    
    void setSampleRate(float newSampleRate) override {
        sampleRate = newSampleRate;
    }
    
    float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
    float accentGain = 0.8f + accent * 0.6f;
        
    const float toneFreq = 300.0f;
        
        // Generate tone component
        tonePhase += toneFreq * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase >= 2.0f * clonotribe::FastMath::PI) {
            tonePhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float tone = clonotribe::FastMath::fastSin(tonePhase) * toneEnv;
        
        // Generate bright, crispy noise
        float rawNoise = noise.process();
        
    const float hpCutoff = 0.14f;
        highpassState += (rawNoise - highpassState) * hpCutoff;
        float brightNoise = (rawNoise - highpassState) * noiseEnv;
        
    const float bpCutoff = 0.22f;
        bandpassState1 += (brightNoise - bandpassState1) * bpCutoff;
        bandpassState2 += (bandpassState1 - bandpassState2) * bpCutoff;
        float crackNoise = (bandpassState1 - bandpassState2) * cracklEnv;
        
    const float crackleCutoff = 0.38f;
        crackleFilter += (crackNoise - crackleFilter) * crackleCutoff;
        float textureNoise = crackleFilter;
        
    float output = tone * 0.2f + brightNoise * 0.55f + crackNoise * 0.75f + textureNoise * 0.25f;
        
    toneEnv *= 0.9945f;
    noiseEnv *= 0.9875f;
    cracklEnv *= 0.9915f;
    ampEnv *= 0.9905f;
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        output *= ampEnv;
    output = clonotribe::FastMath::fastTanh(output * 2.6f);
        
    return output * 1.35f * accentGain;
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