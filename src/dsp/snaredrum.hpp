#pragma once
#include <rack.hpp>
#include "fastmath.hpp"
#include "noise.hpp"

using namespace rack;

namespace clonotribe {


struct SnareDrum {
    // Constants
    static constexpr float kBodyCutoff = 0.15f;
    static constexpr float kBuzzCutoff = 0.6f;
    static constexpr float kCrackCutoff = 0.85f;
    static constexpr float kBodyFreq = 200.0f;
    static constexpr float kHarmonicFreq = 285.0f;
    static constexpr float kBodyResonance = 2.5f;
    static constexpr float kOutputGain = 4.2f;

    // Multiple components for authentic snare sound
    float tonePhase1 = 0.0f;
    float tonePhase2 = 0.0f;
    float noiseState = 12345.0f;
    float envelope = 0.0f;
    float toneEnv = 0.0f;
    float buzzEnv = 0.0f;
    bool triggered = false;

    // Multiple filters for different frequency ranges
    float bodyFilter1 = 0.0f, bodyFilter2 = 0.0f;
    float buzzFilter1 = 0.0f, buzzFilter2 = 0.0f;
    float crackFilter1 = 0.0f, crackFilter2 = 0.0f;
    float noiseFilter = 0.0f;

    void trigger() noexcept {
        envelope = 1.0f;
        toneEnv = 1.0f;
        buzzEnv = 1.0f;
        triggered = true;
        tonePhase1 = 0.0f;
        tonePhase2 = 0.0f;
        noiseState = static_cast<float>(random::u32());
    }

    [[nodiscard]] float process(float sampleTime, NoiseGenerator& noise) noexcept {
        if (!triggered) return 0.0f;

        // Generate high-quality noise from shared source
        float noiseValue = noise.process();

        // Pre-filter noise for better character
        noiseFilter = noiseFilter * 0.92f + noiseValue * 0.08f;
        float coloredNoise = noiseValue - noiseFilter;

        // Drum body tone (fundamental frequencies)
        float freq1 = kBodyFreq * (1.0f + toneEnv * toneEnv * 0.8f); // main body frequency with pitch bend
        float freq2 = kHarmonicFreq * (1.0f + toneEnv * 0.4f);       // harmonic frequency

        
        tonePhase1 += freq1 * sampleTime * 2.0f * FastMath::PI;
        if (tonePhase1 >= 2.0f * FastMath::PI) tonePhase1 -= 2.0f * FastMath::PI;

        tonePhase2 += freq2 * sampleTime * 2.0f * FastMath::PI;
        if (tonePhase2 >= 2.0f * FastMath::PI) tonePhase2 -= 2.0f * FastMath::PI;

        float tone1 = FastMath::fastSin(tonePhase1);
        float tone2 = FastMath::fastSin(tonePhase2) * 0.7f;

        // Body resonance filter (bandpass around fundamental)
        float bodyInput = tone1 + tone2;
        bodyFilter1 += (bodyInput - bodyFilter1) * kBodyCutoff;
        bodyFilter2 += (bodyFilter1 - bodyFilter2) * kBodyCutoff;
        float bodyTone = bodyInput + (bodyFilter1 - bodyFilter2) * kBodyResonance; // Resonance

        // Classic snare "buzz" (filtered noise with specific character)
        buzzFilter1 += (coloredNoise - buzzFilter1) * kBuzzCutoff;
        buzzFilter2 += (buzzFilter1 - buzzFilter2) * kBuzzCutoff;
        float buzzNoise = buzzFilter1 - buzzFilter2 * 0.5f;

        // High-frequency "crack" component with more metallic character
        crackFilter1 += (noiseValue - crackFilter1) * kCrackCutoff;
        crackFilter2 += (crackFilter1 - crackFilter2) * kCrackCutoff;
        float crackNoise = crackFilter1 - crackFilter2 * 0.3f;

        // Additional metallic noise component (higher frequency)
        float metallicNoise = noiseValue * noiseValue * noiseValue; // cube for more harmonic content
        metallicNoise = FastMath::fastTanh(metallicNoise * 3.0f); // heavy saturation for metallic character

        // Different envelope shapes for different components
        toneEnv -= sampleTime * 6.0f;      // medium decay for body tone
        buzzEnv -= sampleTime * 4.0f;      // slower decay for buzz (characteristic snare sustain)
        envelope -= sampleTime * 8.0f;     // fast decay for crack

        if (envelope <= 0.0f) {
            envelope = 0.0f;
            toneEnv = 0.0f;
            buzzEnv = 0.0f;
            triggered = false;
        }

        // Clamp envelopes
        toneEnv = clamp(toneEnv, 0.0f, 1.0f);
        buzzEnv = clamp(buzzEnv, 0.0f, 1.0f);

        // Mix components with classic snare proportions
        float bodyComponent = bodyTone * toneEnv * toneEnv * 0.4f;
        float buzzComponent = buzzNoise * buzzEnv * 0.8f;
        float crackComponent = crackNoise * envelope * envelope * 0.3f;
        float metallicComponent = metallicNoise * envelope * 0.2f;
        float output = bodyComponent + buzzComponent + crackComponent + metallicComponent;

        // Gentle saturation for analog character
        output = FastMath::fastTanh(output * 1.8f) * 0.7f;

        return output * kOutputGain;
    }
};
}