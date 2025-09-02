#pragma once
#include <rack.hpp>
#include <array>
#include <algorithm>
#include "fastmath.hpp"

namespace clonotribe {

class LFO final {
    static constexpr float SLOW_MIN_HZ = 0.05f;
    static constexpr float SLOW_MAX_HZ = 18.0f;
    static constexpr float FAST_MIN_HZ = 1.0f;
    static constexpr float FAST_MAX_HZ = 5000.0f;

public:
    enum class Waveform {
        SQUARE = 0,
        TRIANGLE = 1,
        SAW = 2,
        SAMPLE_HOLD = 3
    };

    enum class Mode {
        ONE_SHOT = 0,
        SLOW = 1,
        FAST = 2
    };

    enum class Target {
        VCF = 0,
        VCO_VCF = 1,
        VCO = 2
    };

    constexpr LFO() noexcept = default;
    LFO(const LFO&) noexcept = default;
    LFO& operator=(const LFO&) noexcept = default;
    LFO(LFO&&) noexcept = default;
    LFO& operator=(LFO&&) noexcept = default;
    ~LFO() noexcept = default;

    [[nodiscard]] float process(Mode mode, float rate, bool externalCV, bool rising, Waveform waveform, float intensity, float sampleTime) {
        update(mode, rate, externalCV, rising);
        return processInternal(sampleTime, waveform) * intensity;
    }

    void setSampleAndHold(bool sh) noexcept {
        sampleAndHold = sh;
        if (sh) {
            sampleHoldValue = (rack::random::uniform() - HALF) * TWO;
        }
    }

    void trigger() noexcept {
        if (oneShot) {
            phase = ZERO;
            triggered = true;
        }
    }

    void reset(float gate) {
        static bool prevGate = false;
        bool noteOn = (gate > HALF) && !prevGate;
        if (noteOn) {
            trigger();
        }
        prevGate = (gate > HALF);
    }

     [[nodiscard]] bool phaseWrapped() noexcept {
        bool wrapped = (phase < previousPhase);
        previousPhase = phase;
        return wrapped;
    }

    void setActive(bool active) noexcept {
        this->active = active;
    }

private:
    float phase = ZERO;
    float previousPhase = ZERO;
    float freq = ONE;
    float sampleHoldValue = ZERO;
    float lastPhase = ZERO;
    bool oneShot = false;
    bool triggered = false;
    bool active = true;
    bool sampleAndHold = false;

    void update(Mode mode, float rate, bool rateCVConnected, bool gateRising) {
        if (rateCVConnected) {
            freq = rate;
        } else {
            float minHz, maxHz;
            switch (mode) {
                case Mode::SLOW:
                    minHz = SLOW_MIN_HZ;
                    maxHz = SLOW_MAX_HZ;
                    oneShot = false;
                    break;
                case Mode::FAST:
                    minHz = FAST_MIN_HZ;
                    maxHz = FAST_MAX_HZ;
                    oneShot = false;
                    break;
                case Mode::ONE_SHOT:
                default:
                    minHz = SLOW_MIN_HZ;
                    maxHz = SLOW_MAX_HZ;
                    oneShot = true;
                    break;
            }
            freq = minHz * std::pow(maxHz / minHz, std::clamp(rate, ZERO, ONE));
        }
        if (gateRising && (mode == Mode::FAST || mode == Mode::ONE_SHOT)) {
            trigger();
        }
    }

    [[nodiscard]] float processInternal(float sampleTime, Waveform waveform = Waveform::SQUARE) noexcept {
        if (!active || (oneShot && !triggered)) [[unlikely]] {
            return ZERO;
        }

        phase += freq * sampleTime;
        if (oneShot && phase >= HALF) {
            triggered = false;
            return ZERO;
        }
        if (phase >= ONE) {
            phase -= ONE;
            if (oneShot) {
                triggered = false;
            }
        }

        float output = ZERO;

        if (sampleAndHold) {
            if (phase < lastPhase) {
                sampleHoldValue = (rack::random::uniform() - HALF) * TWO;
            }
            output = sampleHoldValue;
        } else {
            switch (waveform) {
                case Waveform::SQUARE:
                    output = (phase < HALF) ? ONE : -ONE;
                    break;
                case Waveform::TRIANGLE:
                    if (phase < HALF) {
                        output = 4.0f * phase - ONE;
                    } else {
                        output = 3.0f - 4.0f * phase;
                    }
                    break;
                case Waveform::SAW:
                    output = TWO * phase - ONE;
                    break;
                case Waveform::SAMPLE_HOLD:
                    if (phase < lastPhase) {
                        sampleHoldValue = (rack::random::uniform() - HALF) * TWO;
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