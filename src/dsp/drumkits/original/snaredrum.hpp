#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        ampEnv = 1.0f;
        toneEnv = 1.0f;
        noiseEnv = 1.0f;
        buzzEnv = 1.0f;
        tonePhase1 = 0.0f;
        tonePhase2 = 0.0f;
        noiseFilter1 = 0.0f;
        noiseFilter2 = 0.0f;
        bodyFilter = 0.0f;
        hpState = 0.0f;
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
        float accentGain = 0.75f + accent * 0.5f;
        
        tonePhase1 += FREQ1 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase1 >= 2.0f * clonotribe::FastMath::PI) {
            tonePhase1 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        tonePhase2 += FREQ2 * invSampleRate * 2.0f * clonotribe::FastMath::PI;
        if (tonePhase2 >= 2.0f * clonotribe::FastMath::PI) {
            tonePhase2 -= 2.0f * clonotribe::FastMath::PI;
        }
        
        float tone1 = clonotribe::FastMath::fastSin(tonePhase1) * toneEnv;
        float tone2 = clonotribe::FastMath::fastSin(tonePhase2) * toneEnv * 0.6f;
        
        float toneSum = tone1 + tone2;
        bodyFilter += (toneSum - bodyFilter) * 0.7f;
        
        float rawNoise = noise.process();
        
        noiseFilter1 += (rawNoise - noiseFilter1) * CUTOFF_1;
        noiseFilter2 += (noiseFilter1 - noiseFilter2) * CUTOFF_2;
        float buzzNoise = (noiseFilter1 - noiseFilter2) * buzzEnv;
        
        hpState += (buzzNoise - hpState) * HP_CUTOFF;
        buzzNoise -= hpState;

        float bodyTone = bodyFilter * 0.45f;
        float snareNoise = buzzNoise * 0.75f;
        float output = bodyTone + snareNoise;

        toneEnv *= 0.9935f;
        buzzEnv *= 0.985f;
        noiseEnv *= 0.988f;
        ampEnv *= 0.990f;
        
        if (ampEnv < 0.001f && buzzEnv < 0.001f) {
            triggered = false;
        }
        
        output *= ampEnv;
        output = clonotribe::FastMath::fastTanh(output * 1.6f);
        
        return output * 1.4f * accentGain;
    }
    
private:
    static constexpr float HP_CUTOFF = 0.05f;
    static constexpr float CUTOFF_1 = 0.28f;
    static constexpr float CUTOFF_2 = 0.18f;
    static constexpr float FREQ1 = 210.0f;
    static constexpr float FREQ2 = 330.0f;

    float ampEnv = 0.0f;
    float toneEnv = 0.0f;
    float noiseEnv = 0.0f;
    float buzzEnv = 0.0f;
    float tonePhase1 = 0.0f;
    float tonePhase2 = 0.0f;
    float noiseFilter1 = 0.0f;
    float noiseFilter2 = 0.0f;
    float bodyFilter = 0.0f;
    float hpState = 0.0f;
    float sampleRate = 44100.0f;
    bool triggered = false;
};

}
}