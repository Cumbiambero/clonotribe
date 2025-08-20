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
            pitch = 0.0f;
        }
        pitch = std::clamp(pitch, -10.0f, 10.0f);
        freq = rack::dsp::FREQ_C4 * std::pow(2.0f, pitch);
        if (!std::isfinite(freq)) {
            freq = rack::dsp::FREQ_C4;
        }
        freq = std::clamp(freq, 0.1f, 48000.0f);
        active = freq > 1.0f;
    }

    void setPulseWidth(float pw) noexcept {
        pulseWidth = std::clamp(pw, 0.01f, 0.99f);
    }

private:
    float phase{0.0f};
    float freq{440.0f};
    float pulseWidth{0.5f};
    float lastSaw{0.0f};
    float lastPulse{0.0f};
    bool active{true};
    
    Waveform currentWaveform{Waveform::SAW};
    float (VCO::*processFunction)(float){&VCO::processSaw};

    rack::dsp::RCFilter antiAlias{};

    [[nodiscard]] static constexpr float polyBLEP(float t, float dt) noexcept {
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
        if (!active) {
            return  0.0f;
        }

        const auto dt = freq * sampleTime;
        if (dt > 1.0f) {
            phase = 0.0f;
            return lastSaw;
        }
        
        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        auto saw = 2.0f * phase - 1.0f;
        saw -= polyBLEP(phase, dt);

        lastSaw = saw;
        return saw;
    }


    [[nodiscard]] float processTriangle(float sampleTime) noexcept {
        if (!active) {
            return 0.0f;
        }

        const auto dt = freq * sampleTime;
        if (dt > 1.0f) {
            phase = 0.0f;
            return 0.0f;
        }

        phase += dt;
        if (phase >= 1.0f) phase -= 1.0f;

        float triangle;
        if (phase < 0.5f) {
            triangle = 4.0f * phase - 1.0f;
        } else {
            triangle = 3.0f - 4.0f * phase;
        }

        float shaped = triangle * triangle * triangle;
        triangle = triangle + 0.05f * shaped;
        
        return triangle;
    }

    [[nodiscard]] float processSquare(float sampleTime) noexcept {
        if (!active) {
            return 0.0f;
        }

        const auto dt = freq * sampleTime;
        if (dt > 1.0f) {
            phase = 0.0f;
            return 0.0f;
        }

        phase += dt;
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

        return square;
    }
};
}