#pragma once
#include <rack.hpp>
#include <cmath>
#include "fastmath.hpp"

namespace clonotribe {

class VCO final {
public:
    enum class Waveform {
        SQUARE = 0,
        TRIANGLE = 1,
        SAW = 2
    };

    constexpr VCO() noexcept = default;

    void initialize() noexcept {
        antiAlias.setCutoff(8000.0f / 48000.0f);
        setWaveform(currentWaveform);
    }

    void setWaveform(Waveform waveform) noexcept {
        if (currentWaveform != waveform) {
            currentWaveform = waveform;
            switch (waveform) {
                case Waveform::TRIANGLE: 
                    processFunction = &VCO::processTriangle; 
                    break;
                case Waveform::SAW: 
                    processFunction = &VCO::processSaw; 
                    break;
                case Waveform::SQUARE: 
                default: 
                    processFunction = &VCO::processSquare; 
            }
        }
    }

    [[nodiscard]] float process(float sampleTime) noexcept {
        return (this->*processFunction)(sampleTime);
    }

    void setPitch(float pitch) noexcept {
        if (!std::isfinite(pitch)) {
            pitch = ZERO;
        }
        pitch = std::clamp(pitch, -10.0f, 10.0f);
        freq = rack::dsp::FREQ_C4 * std::pow(TWO, pitch);
        if (!std::isfinite(freq)) {
            freq = rack::dsp::FREQ_C4;
        }
        freq = std::clamp(freq, 0.1f, 48000.0f);
        active = freq > ONE;
    }

    void setPulseWidth(float pw) noexcept {
        pulseWidth = std::clamp(pw, 0.01f, 0.99f);
    }

private:
    float phase{ZERO};
    float freq{440.0f};
    float pulseWidth{0.5f};
    float lastSaw{ZERO};
    float lastPulse{ZERO};
    bool active{true};
    
    Waveform currentWaveform{Waveform::SAW};
    float (VCO::*processFunction)(float){&VCO::processSaw};

    rack::dsp::RCFilter antiAlias{};

    [[nodiscard]] static constexpr float polyBLEP(float t, float dt) noexcept {
        if (t < dt) {
            t /= dt;
            return t + t - t * t - ONE;
        } else if (t > ONE - dt) {
            t = (t - ONE) / dt;
            return t * t + t + t + ONE;
        } else {
            return ZERO;
        }
    }

    [[nodiscard]] float processSaw(float sampleTime) noexcept {
        if (!active) {
            return  ZERO;
        }

        const auto dt = freq * sampleTime;
        if (dt > ONE) {
            phase = ZERO;
            return lastSaw;
        }
        
        phase += dt;
        if (phase >= ONE) phase -= ONE;

        auto saw = TWO * phase - ONE;
        saw -= polyBLEP(phase, dt);

        lastSaw = saw;
        return saw;
    }


    [[nodiscard]] float processTriangle(float sampleTime) noexcept {
        if (!active) {
            return ZERO;
        }

        const auto dt = freq * sampleTime;
        if (dt > ONE) {
            phase = ZERO;
            return ZERO;
        }

        phase += dt;
        if (phase >= ONE) phase -= ONE;

        float triangle;
        if (phase < 0.5f) {
            triangle = 4.0f * phase - ONE;
        } else {
            triangle = 3.0f - 4.0f * phase;
        }

        float shaped = triangle * triangle * triangle;
        triangle = triangle + 0.05f * shaped;
        
        return triangle;
    }

    [[nodiscard]] float processSquare(float sampleTime) noexcept {
        if (!active) {
            return ZERO;
        }

        const auto dt = freq * sampleTime;
        if (dt > ONE) {
            phase = ZERO;
            return ZERO;
        }

        phase += dt;
        if (phase >= ONE) phase -= ONE;

        float square = (phase < 0.5f) ? ONE : -ONE;

        constexpr float transition = 0.005f;
        if (phase > 0.5f - transition && phase < 0.5f + transition) {
            float t = (phase - (0.5f - transition)) / (TWO * transition);
            square = ONE - TWO * t;
        } else if (phase < transition) {
            float t = phase / transition;
            square = -ONE + TWO * t;
        } else if (phase > ONE - transition) {
            float t = (phase - (ONE - transition)) / transition;
            square = ONE - TWO * t;
        }

        return square;
    }
};
}