#pragma once
#include <numbers>
#include "fastmath.hpp"

namespace clonotribe {

struct KickDrum final {
    float phase = 0.0f;
    float envelope = 0.0f;
    bool triggered = false;

    constexpr KickDrum() noexcept = default;
    KickDrum(const KickDrum&) noexcept = default;
    KickDrum& operator=(const KickDrum&) noexcept = default;
    KickDrum(KickDrum&&) noexcept = default;
    KickDrum& operator=(KickDrum&&) noexcept = default;
    ~KickDrum() noexcept = default;

    void trigger() noexcept {
        phase = 0.0f;
        envelope = 1.0f;
        triggered = true;
    }

    [[nodiscard]] float process(float sampleTime) noexcept {
        if (!triggered) [[unlikely]] return 0.0f;

        float pitchEnv = envelope * envelope * envelope * 45.0f + 35.0f;
        phase += pitchEnv * sampleTime * 2.0f * FastMath::PI;
        if (phase >= 2.0f * FastMath::PI) phase -= 2.0f * FastMath::PI;

        float sine = FastMath::fastSin(phase);
        float subPhase = phase * 0.5f;
        float subSine = FastMath::fastSin(subPhase) * 0.4f;
        float click = (envelope > 0.9f) ? (envelope - 0.9f) * 10.0f : 0.0f;

        envelope -= sampleTime * 2.5f;
        if (envelope <= 0.0f) {
            envelope = 0.0f;
            triggered = false;
        }

        return (sine + subSine + click * 0.2f) * envelope * envelope * 4.0f;
    }
};
}