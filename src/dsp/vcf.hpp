#pragma once
#include "fastmath.hpp"

namespace clonotribe {

struct VCF final {
    float cutoff = 1000.0f;
    float resonance = 0.5f;
    bool active = true;

    // State variables for 4 stages
    float s[4] = {0.0f, 0.0f, 0.0f, 0.0f};

    // Nonlinearity strength per stage
    float nonlin[4] = {1.0f, 1.0f, 1.0f, 1.0f};

    constexpr VCF() noexcept = default;
    VCF(const VCF&) noexcept = default;
    VCF& operator=(const VCF&) noexcept = default;
    VCF(VCF&&) noexcept = default;
    VCF& operator=(VCF&&) noexcept = default;
    ~VCF() noexcept = default;

    void setActive(bool a) noexcept { active = a; }
    void setCutoff(float freq) noexcept {
        cutoff = freq < 20.0f ? 20.0f : (freq > 20000.0f ? 20000.0f : freq);
    }
    void setResonance(float res) noexcept {
        resonance = res < 0.0f ? 0.0f : (res > 2.5f ? 2.5f : res);
    }
    void setNonlinearity(float n0, float n1, float n2, float n3) noexcept {
        nonlin[0] = n0; nonlin[1] = n1; nonlin[2] = n2; nonlin[3] = n3;
    }
    void reset() noexcept {
        for (int i = 0; i < 4; ++i) s[i] = 0.0f;
    }

    // Internal process for one sample (used for oversampling)
    float processSample(float input, float g, float k, float input_gain) noexcept {
        // Diode nonlinearity in feedback path (use last stage)
        float diode_fb = FastMath::fastTanh(s[3] * nonlin[3]);

        // Feedback modulated by input amplitude (MS-20 "flaw")
        float feedback = k * diode_fb * (1.0f + 0.2f * FastMath::fastTanh(std::abs(input)));

        // Four nonlinear integrator stages
        float stage_in = input * input_gain - feedback;
        for (int i = 0; i < 4; ++i) {
            float x = (i == 0 ? stage_in : s[i-1]) - s[i];
            // Nonlinearity per stage for "diode ladder" effect
            x = FastMath::fastTanh(x * nonlin[i]);
            s[i] += x * g;
        }

        // Output: soft clip for analog character
        return FastMath::fastTanh(s[3]);
    }

    // MS-20 VCF process (diode ladder, 4-stage, nonlinear feedback, 2x oversampling)
    [[nodiscard]] float process(float input, float sampleRate) noexcept {
        if (!active) return input;
        if (std::abs(input) < 1e-6f) {
            reset();
            return 0.0f;
        }

        // Clamp cutoff
        float fc = cutoff;
        if (fc < 20.0f) fc = 20.0f;
        if (fc > 0.45f * sampleRate) fc = 0.45f * sampleRate;

        // Calculate integrator coefficient
        float wc = 2.0f * FastMath::PI * fc / sampleRate;
        float g = wc / (1.0f + wc);

        // Resonance mapping: MS-20 self-oscillates at high resonance
        float k = resonance * 2.0f;

        // Input gain compensation (MS-20 input stage is hot)
        float input_gain = 1.5f;

        // Simple 2x oversampling (process twice with half input each time)
        float out1 = processSample(input * 0.5f, g, k, input_gain);
        float out2 = processSample(input * 0.5f, g, k, input_gain);

        // Average the two outputs
        return 0.5f * (out1 + out2);
    }
};

}