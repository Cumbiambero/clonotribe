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
    
    [[nodiscard]] float process(float trig, float accent, clonotribe::NoiseGenerator& noise) override {
        if (!triggered) {
            return 0.0f;
        }
        
        float invSampleRate = 1.0f / sampleRate;
        float accentGain = 0.8f + accent * 0.6f;
        
        tonePhase += FREQ * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase >= 2.0f * clonotribe::FastMath::PI) {
            tonePhase -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float tone = clonotribe::FastMath::fastSin(tonePhase) * toneEnv;
        float rawNoise = noise.process();
        
        highpassState += (rawNoise - highpassState) * HP_CUTOFF;
        float brightNoise = (rawNoise - highpassState) * noiseEnv;

        bandpassState1 += (brightNoise - bandpassState1) * BP_CUTOFF;
        bandpassState2 += (bandpassState1 - bandpassState2) * BP_CUTOFF;
        float crackNoise = (bandpassState1 - bandpassState2) * cracklEnv;
        
        crackleFilter += (crackNoise - crackleFilter) * CRACKLE_CUTOFF;
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
    static constexpr float FREQ = 300.0f;
    static constexpr float HP_CUTOFF = 0.14f;
    static constexpr float BP_CUTOFF = 0.22f;
    static constexpr float CRACKLE_CUTOFF = 0.38f;

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