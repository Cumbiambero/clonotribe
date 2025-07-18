#pragma once
#include <rack.hpp>

namespace clonotribe {

struct Envelope final {
    enum class Stage { Attack, Decay, Sustain, Release, Off };
    Stage stage = Stage::Off;
    float value = 0.0f;
    float attack = 0.1f;
    float decay = 0.1f;
    float sustain = 0.7f;
    float releaseTime = 0.5f;

    void setAttack(float a) noexcept { attack = std::clamp(a, 0.001f, 10.0f); }
    void setDecay(float d) noexcept { decay = std::clamp(d, 0.001f, 10.0f); }
    void setSustain(float s) noexcept { sustain = std::clamp(s, 0.0f, 1.0f); }
    void setRelease(float r) noexcept { releaseTime = std::clamp(r, 0.001f, 10.0f); }

    void trigger() noexcept { stage = Stage::Attack; }

    void gateOff() noexcept {
        if (stage != Stage::Off) stage = Stage::Release;
    }

    [[nodiscard]] float process(float sampleTime) noexcept {
        switch (stage) {
            case Stage::Attack:
                value += sampleTime / attack;
                if (value >= 1.0f) {
                    value = 1.0f;
                    stage = Stage::Decay;
                }
                break;
            case Stage::Decay:
                value -= sampleTime / decay;
                if (value <= sustain) {
                    value = sustain;
                    stage = Stage::Sustain;
                }
                break;
            case Stage::Sustain:
                value = sustain;
                break;
            case Stage::Release:
                value -= sampleTime / releaseTime;
                if (value <= 0.0f) {
                    value = 0.0f;
                    stage = Stage::Off;
                }
                break;
            case Stage::Off:
                value = 0.0f;
                break;
        }
        return value;
    }
};
}