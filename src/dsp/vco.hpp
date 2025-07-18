#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

namespace clonotribe {

struct VCO final {
    float phase{0.0f};
    float freq{440.0f};
    float pulseWidth{0.5f};
    float lastSaw{0.0f};
    float lastPulse{0.0f};
    bool active{true};

    rack::dsp::RCFilter antiAlias{};
    float dcBlockerY{0.0f};
    float dcBlockerX{0.0f};
    static constexpr float dcBlockerAlpha = 0.995f;

    constexpr VCO() noexcept = default;

    void initialize() noexcept {
        antiAlias.setCutoff(8000.0f / 48000.0f); // 8kHz cutoff
    }

    void setPitch(float pitch) noexcept {
        freq = rack::dsp::FREQ_C4 * std::pow(2.0f, static_cast<float>(pitch));
        active = freq > 1.0f;
    }

    void setPulseWidth(float pw) noexcept {
        pulseWidth = std::clamp(pw, 0.01f, 0.99f);
    }

    // PolyBLEP helper for band-limited step
    [[nodiscard]] static constexpr float polyblep(float t, float dt) noexcept {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - 1.0f;
        } else if (t > 1.0f - dt) {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        } else {
            return 0.0f;
        }
    }

    [[nodiscard]] float processSaw(float sampleTime) noexcept {
        if (!active) return lastSaw;

        const auto dt = freq * sampleTime;
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        auto saw = 2.0f * phase - 1.0f;
        saw -= polyblep(phase, dt);

        // DC-blocking highpass filter
        float y = saw - dcBlockerX + dcBlockerAlpha * dcBlockerY;
        dcBlockerX = saw;
        dcBlockerY = y;

        lastSaw = y;
        return y;
    }


    [[nodiscard]] float processTriangle(float sampleTime) noexcept {
        if (!active) return 0.0f;

        phase += freq * sampleTime;
        if (phase >= 1.0f) phase -= 1.0f;

        float triangle;
        if (phase < 0.5f) {
            triangle = 4.0f * phase - 1.0f;
        } else {
            triangle = 3.0f - 4.0f * phase;
        }

        triangle = triangle + 0.05f * triangle * triangle * triangle;

        // DC-blocking highpass filter
        float y = triangle - dcBlockerX + dcBlockerAlpha * dcBlockerY;
        dcBlockerX = triangle;
        dcBlockerY = y;

        return y;
    }

    [[nodiscard]] float processSquare(float sampleTime) noexcept {
        if (!active) return 0.0f;

        phase += freq * sampleTime;
        if (phase >= 1.0f) phase -= 1.0f;

        float square = (phase < 0.5f) ? 1.0f : -1.0f;

        constexpr float transition = 0.005f;
        if (phase > 0.5f - transition && phase < 0.5f + transition) {
            float t = (phase - (0.5f - transition)) / (2.0f * transition);
            square = 1.0f - 2.0f * t;
        } else if (phase < transition) {
            float t = phase / transition;
            square = -1.0f + 2.0f * t;
        } else if (phase > 1.0f - transition) {
            float t = (phase - (1.0f - transition)) / transition;
            square = 1.0f - 2.0f * t;
        }

        // DC-blocking highpass filter
        float y = square - dcBlockerX + dcBlockerAlpha * dcBlockerY;
        dcBlockerX = square;
        dcBlockerY = y;

        return y;
    }
};
}