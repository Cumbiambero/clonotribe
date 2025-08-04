#pragma once
#include "fastmath.hpp"
#include "noise.hpp"

namespace clonotribe {

class VCF {
public:
    VCF() = default;

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
        cutoff *= (1.f + 0.005f * noiseGen.process());
        cutoff = std::clamp(cutoff, 20.f, sampleRate * 0.4f);

        float resonance = calculateResonance(resonanceParam);
        float f = 2.f * FastMath::fastSin(FastMath::PI * cutoff * invSampleRate);
        f = std::clamp(f, 0.f, 0.9f);

        float hp = saturate(input - resonance * s2 - s1);
        s1 += f * saturate(hp);
        s2 += f * saturate(s1);

        float output = s2;
        if (cutoffParam < 0.3f) {
            output *= cutoffParam * 3.33f;
        }

        if (std::abs(output) < 1e-30f) output = 0.f;

        return saturate(output * 1.5f);
    }

    void reset() noexcept {
        s1 = s2 = 0.f;
    }
    
private:
    float s1 = 0.f, s2 = 0.f;
    float cutoffParam = 0.5f;
    float resonanceParam = 0.f;
    float sampleRate = 44100.f;
    float invSampleRate = 1.f/44100.f;
    bool active = true;

    NoiseGenerator noiseGen;

    [[nodiscard]] inline float calculateCutoff(float param) const noexcept {
        param = std::clamp(param, 0.001f, 1.f);
        return 20.f * std::exp(7.0f * param);
    }

    [[nodiscard]] inline float calculateResonance(float param) const noexcept {
        param = std::clamp(param, 0.f, 1.f);
        return param * 5.0f;
    }

    [[nodiscard]] inline float saturate(float x) const noexcept {
        x = std::clamp(x, -5.f, 5.f);
        return x / (1.f + std::abs(x));
    }    
};
}