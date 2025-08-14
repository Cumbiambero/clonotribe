#pragma once
#include <cstdint>
#include <numbers>

namespace clonotribe {

enum class NoiseType {
    WHITE,
    PINK
};

struct NoiseGenerator final {
    uint32_t state = 12345;
    NoiseType noiseType = NoiseType::WHITE;
    
    float pinkState[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t pinkCounter = 0;

    constexpr NoiseGenerator() noexcept = default;
    NoiseGenerator(const NoiseGenerator&) noexcept = default;
    NoiseGenerator& operator=(const NoiseGenerator&) noexcept = default;
    NoiseGenerator(NoiseGenerator&&) noexcept = default;
    NoiseGenerator& operator=(NoiseGenerator&&) noexcept = default;
    ~NoiseGenerator() noexcept = default;

    void setSeed(uint32_t seed) noexcept {
        state = seed;
    }

    void setNoiseType(NoiseType type) noexcept {
        noiseType = type;
    }

    [[nodiscard]] NoiseType getNoiseType() const noexcept {
        return noiseType;
    }

    [[nodiscard]] float generateWhiteNoise() noexcept {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        return static_cast<float>(static_cast<int32_t>(state)) * (1.0f / 2147483648.0f);
    }

    [[nodiscard]] float generatePinkNoise() noexcept {
        float white = generateWhiteNoise();
        
        pinkState[0] = 0.99886f * pinkState[0] + white * 0.0555179f;
        pinkState[1] = 0.99332f * pinkState[1] + white * 0.0750759f;
        pinkState[2] = 0.96900f * pinkState[2] + white * 0.1538520f;
        pinkState[3] = 0.86650f * pinkState[3] + white * 0.3104856f;
        pinkState[4] = 0.55000f * pinkState[4] + white * 0.5329522f;
        
        float pink = pinkState[0] + pinkState[1] + pinkState[2] + pinkState[3] + pinkState[4] + white * 0.0750759f;
        return pink * 0.11f;
    }

    [[nodiscard]] float process() noexcept {
        switch (noiseType) {
            case NoiseType::WHITE:
                return generateWhiteNoise();
            case NoiseType::PINK:
                return generatePinkNoise();
            default:
                return generateWhiteNoise();
        }
    }

    void processStereo(float& left, float& right) noexcept {
        left = process();
        state ^= 0x55555555;
        right = process();
    }

    void reset() noexcept {
        state = 12345u;
        for (int i = 0; i < 5; i++) {
            pinkState[i] = 0.0f;
        }
        pinkCounter = 0;
    }
};
}