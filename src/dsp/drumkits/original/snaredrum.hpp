#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace original {

class SnareDrum : public drumkits::SnareDrum {
public:
    void reset() override {
        ampEnv = ONE;
        toneEnv = ONE;
        noiseEnv = ONE;
        buzzEnv = ONE;
        tonePhase1 = ZERO;
        tonePhase2 = ZERO;
        noiseFilter1 = ZERO;
        noiseFilter2 = ZERO;
        bodyFilter = ZERO;
        hpState = ZERO;
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
        float accentGain = 0.75f + accent * 0.5f;
        
        tonePhase1 += FREQ1 * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (tonePhase1 >= TWO * clonotribe::FastMath::PI) {
            tonePhase1 -= TWO * clonotribe::FastMath::PI;
        }
        
        tonePhase2 += FREQ2 * invSampleRate * TWO * clonotribe::FastMath::PI;
        if (tonePhase2 >= TWO * clonotribe::FastMath::PI) {
            tonePhase2 -= TWO * clonotribe::FastMath::PI;
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

    float ampEnv = ZERO;
    float toneEnv = ZERO;
    float noiseEnv = ZERO;
    float buzzEnv = ZERO;
    float tonePhase1 = ZERO;
    float tonePhase2 = ZERO;
    float noiseFilter1 = ZERO;
    float noiseFilter2 = ZERO;
    float bodyFilter = ZERO;
    float hpState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}