#pragma once
#include <rack.hpp>
#include "noisegenerator.hpp"

using namespace rack;

namespace clonotribe {

struct HiHat {
    // Multiple square wave oscillators for metallic timbre
    float osc1Phase = 0.f;
    float osc2Phase = 0.f;
    float osc3Phase = 0.f;
    float osc4Phase = 0.f;
    float osc5Phase = 0.f;
    float osc6Phase = 0.f;
    
    float noiseState = 12345.f;
    float envelope = 0.f;
    bool triggered = false;
    
    // Band-pass filters for metallic resonances
    float bp1State1 = 0.f, bp1State2 = 0.f;
    float bp2State1 = 0.f, bp2State2 = 0.f;
    float bp3State1 = 0.f, bp3State2 = 0.f;
    float hpState = 0.f;
    
    void trigger() {
        envelope = 1.f;
        triggered = true;
        
        // Randomize phases for more natural metallic sound
        osc1Phase = random::uniform() * 2.f * M_PI;
        osc2Phase = random::uniform() * 2.f * M_PI;
        osc3Phase = random::uniform() * 2.f * M_PI;
        osc4Phase = random::uniform() * 2.f * M_PI;
        osc5Phase = random::uniform() * 2.f * M_PI;
        osc6Phase = random::uniform() * 2.f * M_PI;
        
        noiseState = random::u32();
    }
    
    float process(float sampleTime, NoiseGenerator& noise) {
        if (!triggered) return 0.f; // Early exit for performance
        
        // Six square wave oscillators at metallic ratios (like real analog hi-hats)
        // These frequencies create the characteristic metallic timbre
        float freqs[6] = {2127.f, 2833.f, 3371.f, 4253.f, 5659.f, 6761.f};
        float amps[6] = {1.0f, 0.8f, 0.6f, 0.5f, 0.4f, 0.3f};
        
        float metallic = 0.f;
        
        // Generate square waves and mix them
        osc1Phase += freqs[0] * sampleTime * 2.f * M_PI;
        if (osc1Phase >= 2.f * M_PI) osc1Phase -= 2.f * M_PI;
        metallic += (osc1Phase < M_PI ? 1.f : -1.f) * amps[0];
        
        osc2Phase += freqs[1] * sampleTime * 2.f * M_PI;
        if (osc2Phase >= 2.f * M_PI) osc2Phase -= 2.f * M_PI;
        metallic += (osc2Phase < M_PI ? 1.f : -1.f) * amps[1];
        
        osc3Phase += freqs[2] * sampleTime * 2.f * M_PI;
        if (osc3Phase >= 2.f * M_PI) osc3Phase -= 2.f * M_PI;
        metallic += (osc3Phase < M_PI ? 1.f : -1.f) * amps[2];
        
        osc4Phase += freqs[3] * sampleTime * 2.f * M_PI;
        if (osc4Phase >= 2.f * M_PI) osc4Phase -= 2.f * M_PI;
        metallic += (osc4Phase < M_PI ? 1.f : -1.f) * amps[3];
        
        osc5Phase += freqs[4] * sampleTime * 2.f * M_PI;
        if (osc5Phase >= 2.f * M_PI) osc5Phase -= 2.f * M_PI;
        metallic += (osc5Phase < M_PI ? 1.f : -1.f) * amps[4];
        
        osc6Phase += freqs[5] * sampleTime * 2.f * M_PI;
        if (osc6Phase >= 2.f * M_PI) osc6Phase -= 2.f * M_PI;
        metallic += (osc6Phase < M_PI ? 1.f : -1.f) * amps[5];
        
        // Generate filtered noise for sizzle from shared source
        float noiseValue = noise.process();
        
        // High-pass the noise for brightness
        hpState += (noiseValue - hpState) * 0.8f;
        float sizzle = noiseValue - hpState;
        
        // Band-pass filters to emphasize metallic resonances
        float q = 4.f; // Higher Q for more metallic character
        
        // Filter 1: ~3kHz
        bp1State1 += (metallic - bp1State1) * 0.15f;
        bp1State2 += (bp1State1 - bp1State2) * 0.15f;
        float bp1 = (bp1State1 - bp1State2) * q;
        
        // Filter 2: ~5kHz  
        bp2State1 += (metallic - bp2State1) * 0.25f;
        bp2State2 += (bp2State1 - bp2State2) * 0.25f;
        float bp2 = (bp2State1 - bp2State2) * q;
        
        // Filter 3: ~7kHz
        bp3State1 += (metallic - bp3State1) * 0.35f;
        bp3State2 += (bp3State1 - bp3State2) * 0.35f;
        float bp3 = (bp3State1 - bp3State2) * q;
        
        // Mix filtered metallic components
        float filteredMetallic = (bp1 * 0.6f + bp2 * 0.8f + bp3 * 0.4f) * 0.3f;
        
        // Very fast envelope for crisp attack
        envelope -= sampleTime * 18.f;
        if (envelope <= 0.f) {
            envelope = 0.f;
            triggered = false;
        }
        
        // Mix metallic and sizzle components
        float output = filteredMetallic + sizzle * 0.4f;
        
        // Apply envelope with sharp attack characteristic
        float envShaped = envelope * envelope * envelope; // Sharper decay
        
        // Soft saturation for analog character
        output = tanh(output * 2.f) * 0.7f;
        
        return output * envShaped * 4.0f; // Final output level - balanced with other drums
    }
};

}
