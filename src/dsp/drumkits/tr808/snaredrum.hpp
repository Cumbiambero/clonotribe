#pragma once
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
        if (!triggered) return 0.0f;
        
        const float invSampleRate = 1.0f / sampleRate;
        
        // TR-808 snare uses two sine wave oscillators
        const float freq1 = 330.0f; // Primary tone
        const float freq2 = 185.0f; // Secondary tone (slightly detuned)
        
        // Generate tone oscillators
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
        
        // Generate filtered noise for snare buzz
        float rawNoise = noise.process();
        
        // Bandpass filter for snare character (around 1-2kHz)
        const float cutoff = 0.15f; // Normalized frequency
        bandpassState1 += (rawNoise - bandpassState1) * cutoff;
        bandpassState2 += (bandpassState1 - bandpassState2) * cutoff;
        float bandpassOut = bandpassState1 - bandpassState2;
        
        // High-pass filter to remove low frequencies from noise
        const float hpCutoff = 0.05f;
        highpassState += (bandpassOut - highpassState) * hpCutoff;
        float filteredNoise = (bandpassOut - highpassState) * noiseEnv;
        
        // Mix tone and noise components (authentic TR-808 balance)
        float output = toneSum * 0.4f + filteredNoise * 0.8f;
        
        // TR-808 snare envelope characteristics
        toneEnv *= 0.993f;   // Fast tone decay
        noiseEnv *= 0.985f;  // Slower noise decay
        ampEnv *= 0.988f;    // Overall amplitude
        
        if (ampEnv < 0.001f) {
            triggered = false;
        }
        
        // Apply amplitude envelope and soft saturation
        output *= ampEnv;
        output = clonotribe::FastMath::fastTanh(output * 2.0f);
        
        return output * 1.5f;
    }
    
private:
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