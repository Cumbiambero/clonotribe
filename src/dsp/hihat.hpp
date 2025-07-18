#pragma once
#include <numbers>
#include <cmath>
#include "fastmath.hpp"
#include "noise.hpp"

namespace clonotribe {

struct HiHat final {
    float phase = 0.0f;
    float envelope = 0.0f;
    float noiseState = 12345.0f;
    bool triggered = false;

    constexpr HiHat() noexcept = default;
    HiHat(const HiHat&) noexcept = default;
    HiHat& operator=(const HiHat&) noexcept = default;
    HiHat(HiHat&&) noexcept = default;
    HiHat& operator=(HiHat&&) noexcept = default;
    ~HiHat() noexcept = default;

    void trigger() noexcept {
        envelope = 1.0f;
        triggered = true;
        phase = 0.0f;
        noiseState = static_cast<float>(rack::random::u32());
    }

    [[nodiscard]] float process(float sampleTime, NoiseGenerator& noise) noexcept {
        if (!triggered) [[unlikely]] return 0.0f;

        float noiseValue = noise.process();
        float tone = FastMath::fastSin(phase * 2.0f * FastMath::PI) * 0.7f;
        phase += 8000.0f * sampleTime;
        if (phase >= 1.0f) phase -= 1.0f;

        envelope -= sampleTime * 10.0f;
        if (envelope <= 0.0f) {
            envelope = 0.0f;
            triggered = false;
        }

        float output = (tone + noiseValue * 0.5f) * envelope;
        return FastMath::fastTanh(output * 2.0f);
    }
};
}