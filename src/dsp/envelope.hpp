#pragma once
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
    float value = 0.0f;
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float releaseTime = 0.5f;

    void setAttack(float a) noexcept { attack = std::clamp(a, 0.001f, 10.0f); }
    void setDecay(float d) noexcept { decay = std::clamp(d, 0.001f, 10.0f); }
    void setSustain(float s) noexcept { sustain = std::clamp(s, 0.0f, 1.0f); }
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
                if (value >= 1.0f) {
                    value = 1.0f;
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
                if (value <= 0.0f) {
                    value = 0.0f;
                    stage = Stage::OFF;
                }
                break;
            case Stage::OFF:
                value = 0.0f;
                break;
        }
        return value;
    }
};
}