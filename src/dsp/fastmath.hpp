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

    // Optimized sine approximation - most commonly used
    [[nodiscard]] static inline float fastSin(float x) noexcept {        
        // Normalize to [-π, π] more efficiently
        x = x - TWO_PI * std::floor((x + PI) / TWO_PI);
        
        // Taylor series approximation (4th order for speed/accuracy balance)
        const float x2 = x * x;
        return x * (1.0f - x2 * (1.0f/6.0f - x2 * 1.0f/120.0f));
    }

    // Fast cosine using sine
    [[nodiscard]] static inline float fastCos(float x) noexcept {
        return fastSin(x + PI * 0.5f);
    }

    // Highly optimized tanh for saturation (most performance-critical)
    [[nodiscard]] static inline float fastTanh(float x) noexcept {
        // Pade approximation - much faster than std::tanh
        if (x > 2.5f) return 1.0f;
        if (x < -2.5f) return -1.0f;
        
        const float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    // Phase normalization - critical for oscillators  
    [[nodiscard]] static inline float normalizePhase(float phase) noexcept {
        // Efficient phase wrap using conditional instead of fmod
        if (phase >= TWO_PI) {
            return phase - TWO_PI;
        }
        if (phase < 0.0f) {
            return phase + TWO_PI;
        }
        return phase;
    }
    
    // Linear interpolation - useful for tables
    [[nodiscard]] static inline float lerp(float a, float b, float t) noexcept {
        return a + t * (b - a);
    }
    
    // Fast reciprocal approximation  
    [[nodiscard]] static inline float fastInverse(float x) noexcept {
        return 1.0f / x; // Modern CPUs handle this well
    }
};
}