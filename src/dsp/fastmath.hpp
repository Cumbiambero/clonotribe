#pragma once
#include <numbers>
#include <cmath>

namespace clonotribe {

struct FastMath final {
    static constexpr float PI = std::numbers::pi_v<float>;
    static constexpr float TWO_PI = 2.0f * PI;
    static constexpr float INV_PI = 1.0f / PI;

    constexpr FastMath() noexcept = default;
    FastMath(const FastMath&) noexcept = default;
    FastMath& operator=(const FastMath&) noexcept = default;
    FastMath(FastMath&&) noexcept = default;
    FastMath& operator=(FastMath&&) noexcept = default;
    ~FastMath() noexcept = default;

    [[nodiscard]] static inline float fastSin(float x) noexcept {        
        x = x - TWO_PI * std::floor((x + PI) / TWO_PI);
        const float x2 = x * x;
        return x * (1.0f - x2 * (1.0f/6.0f - x2 * 1.0f/120.0f));
    }

    [[nodiscard]] static inline float fastCos(float x) noexcept {
        return fastSin(x + PI * 0.5f);
    }

    [[nodiscard]] static inline float fastTanh(float x) noexcept {
        if (x > 2.5f) return 1.0f;
        if (x < -2.5f) return -1.0f;
        
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    [[nodiscard]] static inline float normalizePhase(float phase) noexcept {
        if (phase >= TWO_PI) {
            return phase - TWO_PI;
        }
        if (phase < 0.0f) {
            return phase + TWO_PI;
        }
        return phase;
    }
    
    [[nodiscard]] static inline float lerp(float a, float b, float t) noexcept {
        return a + t * (b - a);
    }
    
    [[nodiscard]] static inline float fastInverse(float x) noexcept {
        return 1.0f / x;
    }
};
}