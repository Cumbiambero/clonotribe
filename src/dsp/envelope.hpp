#pragma once
#include "../constants.hpp"
#include <rack.hpp>

namespace clonotribe {

struct Envelope final {
    enum class Type {
        ATTACK,
        GATE,
        DECAY
    };

    enum class Stage {
        ATTACK,
        DECAY,
        SUSTAIN,
        RELEASE,
        OFF
    };

    Stage stage = Stage::OFF;
    float value = ZERO;
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float releaseTime = HALF;

    void setAttack(float a) noexcept { attack = std::clamp(a, 0.001f, 10.0f); }
    void setDecay(float d) noexcept { decay = std::clamp(d, 0.001f, 10.0f); }
    void setSustain(float s) noexcept { sustain = std::clamp(s, ZERO, ONE); }
    void setRelease(float r) noexcept { releaseTime = std::clamp(r, 0.001f, 10.0f); }
    void trigger() noexcept { stage = Stage::ATTACK; }
    void gateOff() noexcept {
        if (stage != Stage::OFF) {
            stage = Stage::RELEASE;
        }
    }
    
    [[nodiscard]] float process(float sampleTime) noexcept {
        switch (stage) {
            case Stage::ATTACK:
                value += sampleTime / attack;
                if (value >= ONE) {
                    value = ONE;
                    stage = Stage::DECAY;
                }
                break;
            case Stage::DECAY:
                value -= sampleTime / decay;
                if (value <= sustain) {
                    value = sustain;
                    stage = Stage::SUSTAIN;
                }
                break;
            case Stage::SUSTAIN:
                value = sustain;
                break;
            case Stage::RELEASE:
                value -= sampleTime / releaseTime;
                if (value <= ZERO) {
                    value = ZERO;
                    stage = Stage::OFF;
                }
                break;
            case Stage::OFF:
                value = ZERO;
                break;
        }
        return value;
    }
};
}