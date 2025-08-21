#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        env = ONE;
        osc1Phase = ZERO;
        osc2Phase = ZERO;
        osc3Phase = ZERO;
        osc4Phase = ZERO;
        osc5Phase = ZERO;
        osc6Phase = ZERO;
        bandpass1State1 = ZERO;
        bandpass1State2 = ZERO;
        bandpass2State1 = ZERO;
        bandpass2State2 = ZERO;
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
        float accentGain = 0.75f + accent * 0.7f;
        float phases[6] = {osc1Phase, osc2Phase, osc3Phase, osc4Phase, osc5Phase, osc6Phase};
        float squareSum = ZERO;
        
        for (int i = 0; i < 6; i++) {
            phases[i] += FREQUENCIES[i] * invSampleRate * TWO * clonotribe::FastMath::PI;
            if (phases[i] >= TWO * clonotribe::FastMath::PI) {
                phases[i] -= TWO * clonotribe::FastMath::PI;
            }
            
            float square = (phases[i] < clonotribe::FastMath::PI) ? ONE : -ONE;
            squareSum += square * (ONE / 6.0f);
        }
        
        osc1Phase = phases[0]; osc2Phase = phases[1]; osc3Phase = phases[2];
        osc4Phase = phases[3]; osc5Phase = phases[4]; osc6Phase = phases[5];
        
        bandpass1State1 += (squareSum - bandpass1State1) * BP1_CUTOFF;
        bandpass1State2 += (bandpass1State1 - bandpass1State2) * BP1_CUTOFF;
        float bp1Out = bandpass1State1 - bandpass1State2;
        
        bandpass2State1 += (bp1Out - bandpass2State1) * BP2_CUTOFF;
        bandpass2State2 += (bandpass2State1 - bandpass2State2) * BP2_CUTOFF;
        float bp2Out = bandpass2State1 - bandpass2State2;
        
        highpassState += (bp2Out - highpassState) * HP_CUTOFF;
        float filteredSignal = bp2Out - highpassState;        
        float noiseComponent = noise.process() * 0.12f * env;
        float output = (filteredSignal + noiseComponent) * env;
        
        env *= 0.9905f;
        
        if (env < 0.001f) {
            triggered = false;
        }
        
         output = clonotribe::FastMath::fastTanh(output * 2.8f);
         return output * 0.85f * accentGain;
    }
    
private:
    static constexpr float FREQUENCIES[6] = {418.0f, 539.0f, 707.0f, 869.0f, 1131.0f, 1319.0f};
    static constexpr float HP_CUTOFF = 0.07f;
    static constexpr float BP1_CUTOFF = 0.23f;
    static constexpr float BP2_CUTOFF = 0.34f;

    float env = ZERO;
    float osc1Phase = ZERO;
    float osc2Phase = ZERO;
    float osc3Phase = ZERO;
    float osc4Phase = ZERO;
    float osc5Phase = ZERO;
    float osc6Phase = ZERO;
    float bandpass1State1 = ZERO;
    float bandpass1State2 = ZERO;
    float bandpass2State1 = ZERO;
    float bandpass2State2 = ZERO;
    float highpassState = ZERO;
    float sampleRate = 44100.0f;
    bool triggered = false;
};
}
}