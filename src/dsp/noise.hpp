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
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        // Convert to float in range [-1, 1] more efficiently
        return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);
    }

    // Generate stereo noise more efficiently
    void processStereo(float& left, float& right) noexcept {
        // Generate two values with one operation
        left = process();
        state ^= 0x55555555; // Add some decorrelation
        right = process();
    }

    void reset() noexcept {
        state = 12345u;
    }
};
}