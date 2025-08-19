#pragma once
#include "../../fastmath.hpp"
#include "../../noise.hpp"
#include "../base/base.hpp"

namespace drumkits {
namespace tr808 {

class HiHat : public drumkits::HiHat {
public:
    void reset() override {
        env = 1.0f;
        osc1Phase = 0.0f;
        osc2Phase = 0.0f;
        osc3Phase = 0.0f;
        osc4Phase = 0.0f;
        osc5Phase = 0.0f;
        osc6Phase = 0.0f;
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
        float accentGain = 0.75f + accent * 0.7f;
        float phases[6] = {osc1Phase, osc2Phase, osc3Phase, osc4Phase, osc5Phase, osc6Phase};
        float squareSum = 0.0f;
        
        for (int i = 0; i < 6; i++) {
            phases[i] += FREQUENCIES[i] * invSampleRate * 2.0f * clonotribe::FastMath::PI;
            if (phases[i] >= 2.0f * clonotribe::FastMath::PI) {
                phases[i] -= 2.0f * clonotribe::FastMath::PI;
            }
            
            float square = (phases[i] < clonotribe::FastMath::PI) ? 1.0f : -1.0f;
            squareSum += square * (1.0f / 6.0f); // Normalize
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

    float env = 0.0f;
    float osc1Phase = 0.0f;
    float osc2Phase = 0.0f;
    float osc3Phase = 0.0f;
    float osc4Phase = 0.0f;
    float osc5Phase = 0.0f;
    float osc6Phase = 0.0f;
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