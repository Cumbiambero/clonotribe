#pragma once
#include <cstdint>
#include <numbers>

namespace clonotribe {

struct NoiseGenerator final {
    uint32_t state = 12345;

    constexpr NoiseGenerator() noexcept = default;
    NoiseGenerator(const NoiseGenerator&) noexcept = default;
    NoiseGenerator& operator=(const NoiseGenerator&) noexcept = default;
    NoiseGenerator(NoiseGenerator&&) noexcept = default;
    NoiseGenerator& operator=(NoiseGenerator&&) noexcept = default;
    ~NoiseGenerator() noexcept = default;

    void setSeed(uint32_t seed) noexcept {
        state = seed;
    }

    [[nodiscard]] float process() noexcept {
        state = state * 1664525u + 1013904223u;
        return static_cast<float>(state % 65536u) / 32768.0f - 1.0f;
    }

    void processStereo(float& left, float& right) noexcept {
        left = process();
        right = process();
    }

    void reset() noexcept {
        state = 12345u;
    }
};
}