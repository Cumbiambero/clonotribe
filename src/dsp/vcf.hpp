#pragma once
#include <array>
#include <numbers>

namespace clonotribe {

struct VCF final {
    float cutoff = 1000.0f;
    float resonance = 0.5f;
    float state1 = 0.0f;
    float state2 = 0.0f;
    bool active = true;

    constexpr VCF() noexcept = default;
    VCF(const VCF&) noexcept = default;
    VCF& operator=(const VCF&) noexcept = default;
    VCF(VCF&&) noexcept = default;
    VCF& operator=(VCF&&) noexcept = default;
    ~VCF() noexcept = default;

    void setActive(bool a) noexcept {
        active = a;
    }

    void setCutoff(float freq) noexcept {
        cutoff = freq < 20.0f ? 20.0f : (freq > 20000.0f ? 20000.0f : freq);
    }

    void setResonance(float res) noexcept {
        resonance = res < 0.0f ? 0.0f : (res > 4.0f ? 4.0f : res);
    }

    void reset() noexcept {
        state1 = 0.0f;
        state2 = 0.0f;
    }

    [[nodiscard]] float process(float input, float sampleRate) noexcept {
        if (!active) return input;

        constexpr float pi_f = static_cast<float>(std::numbers::pi);
        float omega = 2.0f * pi_f * cutoff / sampleRate;
        float cos_omega = FastMath::fastCos(omega);
        float sin_omega = FastMath::fastSin(omega);

        float feedback = resonance * 0.9f;
        float nonlinearFeedback = FastMath::fastTanh(feedback * state1) * 0.7f;

        input -= nonlinearFeedback;

        // Sallen-Key topology approximation
        float alpha = sin_omega / (2.0f * (1.0f + feedback * 0.1f));

        float b0 = (1.0f - cos_omega) / 2.0f;
        float b1 = 1.0f - cos_omega;
        float b2 = (1.0f - cos_omega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cos_omega;
        float a2 = 1.0f - alpha;

        float output = (b0 * input + b1 * state1 + b2 * state2 - a1 * state1 - a2 * state2) / a0;

        output = FastMath::fastTanh(output * 1.2f) * 0.8f;

        state2 = state1;
        state1 = input;

        return output;
    }
};
}