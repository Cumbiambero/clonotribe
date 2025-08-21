#pragma once
#include "../fastmath.hpp"
#include "../noise.hpp"

namespace clonotribe {

class MS20Filter {
public:
    MS20Filter() = default;

    void setSampleRate(float sr) noexcept {
        sampleRate = std::max(8000.f, sr);
        invSampleRate = 1.f / sampleRate;
        noiseGen.setSeed(static_cast<uint32_t>(sr));
    }

    void setCutoff(float param) noexcept {
        cutoffParam = std::clamp(param, 0.f, 1.f);
    }

    void setResonance(float param) noexcept {
        resonanceParam = std::clamp(param, 0.f, 1.f);
    }

    void setActive(bool isActive) noexcept {
        active = isActive;
        if (!active) reset();
    }

    [[nodiscard]] float process(float input) noexcept {
        if (!active) return 0.f;

        if (std::abs(input) < 1e-30f) input = 0.f;
        if (std::abs(s1) < 1e-30f) s1 = 0.f;
        if (std::abs(s2) < 1e-30f) s2 = 0.f;

        if (!std::isfinite(input)) {
            reset();
            return 0.f;
        }

        float cutoff = calculateCutoff(cutoffParam);
        cutoff = std::clamp(cutoff, 20.f, sampleRate * 0.35f);

        float resonance = calculateResonance(resonanceParam);
        float f = 2.f * FastMath::fastSin(FastMath::PI * cutoff * invSampleRate);
        f = std::clamp(f, 0.f, 0.9f);

        float drive = ONE + resonanceParam * 1.2f;
        
        float drivenInput = input * drive;
        drivenInput = saturate(drivenInput);
        
        float hp = saturate(drivenInput - resonance * s2 - s1);
        
        s1 += f * saturate(hp);
        s2 += f * saturate(s1);

        float output = s2;
        
        if (cutoffParam < 0.4f) {
            if (cutoffParam < 0.3f) {
                output *= ZERO;
            } else {
                float fadeRange = cutoffParam - 0.3f;
                float fadeAmount = fadeRange * 10.0f;
                fadeAmount = fadeAmount * fadeAmount;
                output *= 0.01f + fadeAmount * 0.99f;
            }
        }

        if (resonanceParam > 0.75f) {
            float oscGain = (resonanceParam - 0.75f) * 4.0f;
            oscPhase += TWO * FastMath::PI * cutoff * invSampleRate;
            if (oscPhase >= TWO * FastMath::PI) oscPhase -= TWO * FastMath::PI;
            
            float oscSig = FastMath::fastSin(oscPhase) * oscGain * 0.15f;

            if (cutoff > sampleRate * 0.25f) {
                oscSig *= 0.5f;
            }
            
            output = output * (ONE - oscGain * 0.3f) + oscSig;
        }

        if (std::abs(output) < 1e-30f) output = 0.f;

        float finalGain = 1.1f + resonanceParam * 0.3f;
        output = saturate(output * finalGain);
        
        return output;
    }

    void reset() noexcept {
        s1 = s2 = 0.f;
        oscPhase = 0.f;
    }
    
private:
    float s1 = 0.f, s2 = 0.f;
    float cutoffParam = 0.5f;
    float resonanceParam = 0.f;
    float sampleRate = 44100.f;
    float invSampleRate = 1.f/44100.f;
    float oscPhase = 0.f;
    bool active = true;

    NoiseGenerator noiseGen;

    [[nodiscard]] inline float calculateCutoff(float param) const noexcept {
        param = std::clamp(param, 0.001f, 1.f);
        return 20.f * std::exp(7.0f * param);
    }

    [[nodiscard]] inline float calculateResonance(float param) const noexcept {
        param = std::clamp(param, 0.f, 1.f);
        return param * param * 6.0f + param * 1.5f;
    }

    [[nodiscard]] inline float saturate(float x) const noexcept {
        x = std::clamp(x, -4.f, 4.f);
        if (x > ZERO) {
            return x / (ONE + x * 0.4f);
        } else {
            return x / (ONE + std::abs(x) * 0.5f);
        }
    }    
};
}