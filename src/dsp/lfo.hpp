#pragma once
#include <numbers>
#include <array>
#include "fastmath.hpp"

namespace clonotribe {

struct LFO final {
    enum class Waveform { Square = 0, Triangle = 1, Sawtooth = 2, SampleHold = 3 };
    static constexpr float MIN_FREQ = 0.01f;
    static constexpr float MAX_FREQ = 20.0f;
    static constexpr float MIN_ACTIVE_FREQ = 0.001f;

    float phase = 0.0f;
    float freq = 1.0f;
    bool oneShot = false;
    bool triggered = false;
    bool active = true;
    bool sampleAndHold = false;
    float sampleHoldValue = 0.0f;
    float lastPhase = 0.0f;

    constexpr LFO() noexcept = default;
    LFO(const LFO&) noexcept = default;
    LFO& operator=(const LFO&) noexcept = default;
    LFO(LFO&&) noexcept = default;
    LFO& operator=(LFO&&) noexcept = default;
    ~LFO() noexcept = default;

    void setRate(float rate) noexcept {
        freq = std::clamp(rate, MIN_FREQ, MAX_FREQ);
        active = (freq > MIN_ACTIVE_FREQ);
    }

    void setOneShot(bool os) noexcept {
        oneShot = os;
        if (os && !triggered) {
            phase = 0.0f;
        }
    }

    void setSampleAndHold(bool sh) noexcept {
        sampleAndHold = sh;
        if (sh) {
            sampleHoldValue = (rack::random::uniform() - 0.5f) * 2.0f;
        }
    }

    void trigger() noexcept {
        if (oneShot) {
            phase = 0.0f;
            triggered = true;
        }
    }

    [[nodiscard]] float process(float sampleTime, Waveform waveform = Waveform::Square) noexcept {
        if (!active || (oneShot && !triggered)) [[unlikely]] {
            return 0.0f;
        }

        phase += freq * sampleTime;

        if (oneShot && phase >= 0.5f) {
            triggered = false;
            return 0.0f;
        }

        if (phase >= 1.0f) {
            phase -= 1.0f;
            if (oneShot) {
                triggered = false;
            }
        }

        float output = 0.0f;

        if (sampleAndHold) {
            if (phase < lastPhase) {
                sampleHoldValue = (rack::random::uniform() - 0.5f) * 2.0f;
            }
            output = sampleHoldValue;
        } else {
            switch (waveform) {
                case Waveform::Square:
                    output = (phase < 0.5f) ? 1.0f : -1.0f;
                    break;
                case Waveform::Triangle:
                    if (phase < 0.5f) {
                        output = 4.0f * phase - 1.0f;
                    } else {
                        output = 3.0f - 4.0f * phase;
                    }
                    break;
                case Waveform::Sawtooth:
                    output = 2.0f * phase - 1.0f;
                    break;
                case Waveform::SampleHold:
                    if (phase < lastPhase) {
                        sampleHoldValue = (rack::random::uniform() - 0.5f) * 2.0f;
                    }
                    output = sampleHoldValue;
                    break;
            }
        }

        lastPhase = phase;
        return output;
    }
};
}