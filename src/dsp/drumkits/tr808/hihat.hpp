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
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // TR-808 hi-hat uses 6 square wave oscillators with specific frequencies
        const float freqs[6] = {325.0f, 380.0f, 445.0f, 520.0f, 610.0f, 715.0f};
        float phases[6] = {osc1Phase, osc2Phase, osc3Phase, osc4Phase, osc5Phase, osc6Phase};
        
        float squareSum = 0.0f;
        
        // Generate square waves
        for (int i = 0; i < 6; i++) {
            phases[i] += freqs[i] * invSampleRate * 2.0f * clonotribe::FastMath::PI;
            if (phases[i] >= 2.0f * clonotribe::FastMath::PI) {
                phases[i] -= 2.0f * clonotribe::FastMath::PI;
            }
            
            // Square wave generation
            float square = (phases[i] < clonotribe::FastMath::PI) ? 1.0f : -1.0f;
            squareSum += square * (1.0f / 6.0f); // Normalize
        }
        
        // Update phases
        osc1Phase = phases[0]; osc2Phase = phases[1]; osc3Phase = phases[2];
        osc4Phase = phases[3]; osc5Phase = phases[4]; osc6Phase = phases[5];
        
        // Two bandpass filters for metallic character
        const float bp1Cutoff = 0.25f; // Around 5.5kHz
        const float bp2Cutoff = 0.35f; // Around 7.7kHz
        
        // First bandpass filter
        bandpass1State1 += (squareSum - bandpass1State1) * bp1Cutoff;
        bandpass1State2 += (bandpass1State1 - bandpass1State2) * bp1Cutoff;
        float bp1Out = bandpass1State1 - bandpass1State2;
        
        // Second bandpass filter
        bandpass2State1 += (bp1Out - bandpass2State1) * bp2Cutoff;
        bandpass2State2 += (bandpass2State1 - bandpass2State2) * bp2Cutoff;
        float bp2Out = bandpass2State1 - bandpass2State2;
        
        // High-pass filter for crisp attack
        const float hpCutoff = 0.08f;
        highpassState += (bp2Out - highpassState) * hpCutoff;
        float filteredSignal = bp2Out - highpassState;
        
        // Apply envelope and mix with subtle noise
        float noiseComponent = noise.process() * 0.15f * env;
        float output = (filteredSignal + noiseComponent) * env;
        
        // TR-808 hi-hat decay (quite fast)
        env *= 0.9915f;
        
        if (env < 0.001f) {
            triggered = false;
        }
        
        // Soft saturation for analog character
        output = clonotribe::FastMath::fastTanh(output * 3.0f);
        
        return output * 0.8f;
    }
    
private:
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
