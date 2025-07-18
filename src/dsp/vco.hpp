#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

namespace clonotribe {

struct VCO final {
    float phase = 0.0f;
    float freq = 440.0f;
    float pulseWidth = 0.5f;
    float lastSaw = 0.0f;
    float lastPulse = 0.0f;
    bool active = true;

    rack::dsp::RCFilter antiAlias;

    VCO() noexcept {
        antiAlias.setCutoff(8000.0f / 48000.0f); // 8kHz cutoff
    }

    void setPitch(float pitch) noexcept {
        freq = rack::dsp::FREQ_C4 * std::pow(2.0f, pitch);
        active = (freq > 1.0f);
    }

    void setPulseWidth(float pw) noexcept {
        pulseWidth = std::clamp(pw, 0.01f, 0.99f);
    }

    [[nodiscard]] float processSaw(float sampleTime) noexcept {
        if (!active) return lastSaw;

        phase += freq * sampleTime;
        if (phase >= 1.0f) phase -= 1.0f;

        float saw = 2.0f * phase - 1.0f;
        saw = saw + 0.1f * saw * saw * saw; // add harmonics

        antiAlias.process(saw);
        lastSaw = saw;
        return saw;
    }

    [[nodiscard]] float processPulse(float sampleTime) noexcept {
        if (!active) return lastPulse;

        phase += freq * sampleTime;
        if (phase >= 1.0f) phase -= 1.0f;

        float pulse = (phase < pulseWidth) ? 1.0f : -1.0f;

        constexpr float transition = 0.01f;
        if (phase > pulseWidth - transition && phase < pulseWidth + transition) {
            float t = (phase - (pulseWidth - transition)) / (2.0f * transition);
            pulse = 1.0f - 2.0f * t;
        }

        antiAlias.process(pulse);
        lastPulse = pulse;
        return pulse;
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

        antiAlias.process(triangle);
        return triangle;
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

        antiAlias.process(square);
        return square;
    }
};
}