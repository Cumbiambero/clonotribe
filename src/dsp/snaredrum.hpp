#pragma once
#include <rack.hpp>
#include "fastmath.hpp"
#include "noisegenerator.hpp"

using namespace rack;

namespace clonotribe {

struct SnareDrum {
    // Multiple components for authentic snare sound
    float tonePhase1 = 0.f;
    float tonePhase2 = 0.f;
    float noiseState = 12345.f;
    float envelope = 0.f;
    float toneEnv = 0.f;
    float buzzEnv = 0.f;
    bool triggered = false;
    
    // Multiple filters for different frequency ranges
    float bodyFilter1 = 0.f, bodyFilter2 = 0.f;
    float buzzFilter1 = 0.f, buzzFilter2 = 0.f;
    float crackFilter1 = 0.f, crackFilter2 = 0.f;
    float noiseFilter = 0.f;
    
    void trigger() {
        envelope = 1.f;
        toneEnv = 1.f;
        buzzEnv = 1.f;
        triggered = true;
        tonePhase1 = 0.f;
        tonePhase2 = 0.f;
        noiseState = random::u32();
    }
    
    float process(float sampleTime, NoiseGenerator& noise) {
        if (!triggered) return 0.f; // Early exit for performance
        
        // Generate high-quality noise from shared source
        float noiseValue = noise.process();
        
        // Pre-filter noise for better character
        noiseFilter = noiseFilter * 0.92f + noiseValue * 0.08f;
        float coloredNoise = noiseValue - noiseFilter;
        
        // Drum body tone (fundamental frequencies)
        float freq1 = 200.f * (1.f + toneEnv * toneEnv * 0.8f); // Main body frequency with pitch bend
        float freq2 = 285.f * (1.f + toneEnv * 0.4f);          // Harmonic frequency
        
        tonePhase1 += freq1 * sampleTime * 2.f * M_PI;
        if (tonePhase1 >= 2.f * M_PI) tonePhase1 -= 2.f * M_PI;
        
        tonePhase2 += freq2 * sampleTime * 2.f * M_PI;
        if (tonePhase2 >= 2.f * M_PI) tonePhase2 -= 2.f * M_PI;
        
        float tone1 = FastMath::fastSin(tonePhase1);
        float tone2 = FastMath::fastSin(tonePhase2) * 0.7f;
        
        // Body resonance filter (bandpass around fundamental)
        float bodyInput = tone1 + tone2;
        float cutoff1 = 0.15f;
        bodyFilter1 += (bodyInput - bodyFilter1) * cutoff1;
        bodyFilter2 += (bodyFilter1 - bodyFilter2) * cutoff1;
        float bodyTone = bodyInput + (bodyFilter1 - bodyFilter2) * 2.5f; // Resonance
        
        // Classic snare "buzz" (filtered noise with specific character)
        float cutoff2 = 0.6f;
        buzzFilter1 += (coloredNoise - buzzFilter1) * cutoff2;
        buzzFilter2 += (buzzFilter1 - buzzFilter2) * cutoff2;
        float buzzNoise = buzzFilter1 - buzzFilter2 * 0.5f;
        
        // High-frequency "crack" component with more metallic character
        float cutoff3 = 0.85f;
        crackFilter1 += (noiseValue - crackFilter1) * cutoff3;
        crackFilter2 += (crackFilter1 - crackFilter2) * cutoff3;
        float crackNoise = crackFilter1 - crackFilter2 * 0.3f;
        
        // Additional metallic noise component (higher frequency)
        float metallicNoise = noiseValue * noiseValue * noiseValue; // Cube for more harmonic content
        metallicNoise = FastMath::fastTanh(metallicNoise * 3.f); // Heavy saturation for metallic character
        
        // Different envelope shapes for different components
        toneEnv -= sampleTime * 6.f;      // Medium decay for body tone
        buzzEnv -= sampleTime * 4.f;      // Slower decay for buzz (characteristic snare sustain)
        envelope -= sampleTime * 8.f;     // Fast decay for crack
        
        if (envelope <= 0.f) {
            envelope = 0.f;
            toneEnv = 0.f;
            buzzEnv = 0.f;
            triggered = false;
        }
        
        // Clamp envelopes
        toneEnv = clamp(toneEnv, 0.f, 1.f);
        buzzEnv = clamp(buzzEnv, 0.f, 1.f);
        
        // Mix components with classic snare proportions
        float bodyComponent = bodyTone * toneEnv * toneEnv * 0.4f;           // Punchy body
        float buzzComponent = buzzNoise * buzzEnv * 0.8f;                    // Classic snare buzz
        float crackComponent = crackNoise * envelope * envelope * 0.3f;       // Initial crack/attack
        float metallicComponent = metallicNoise * envelope * 0.2f;           // Metallic character
        
        float output = bodyComponent + buzzComponent + crackComponent + metallicComponent;
        
        // Gentle saturation for analog character
        output = FastMath::fastTanh(output * 1.8f) * 0.7f;
        
        return output * 4.2f; // Final output level - slightly higher for snare prominence
    }
};

}
