#pragma once
#include <numbers>

namespace clonotribe {

struct FastMath final {
    static constexpr float PI = std::numbers::pi_v<float>;

    constexpr FastMath() noexcept = default;
    FastMath(const FastMath&) noexcept = default;
    FastMath& operator=(const FastMath&) noexcept = default;
    FastMath(FastMath&&) noexcept = default;
    FastMath& operator=(FastMath&&) noexcept = default;
    ~FastMath() noexcept = default;

    // Fast sine approximation using polynomial
    [[nodiscard]] static inline float fastSin(float x) noexcept {        
        while (x > PI) x -= 2.f * PI;
        while (x < -PI) x += 2.f * PI;
        float x2 = x * x;
        return x * (1.f - x2 * (1.f/6.f - x2 * 1.f/120.f));
    }

    [[nodiscard]] static inline float fastCos(float x) noexcept {
        return fastSin(x + PI * 0.5f);
    }

    [[nodiscard]] static inline float fastTanh(float x) noexcept {
        x = std::clamp(x, -3.f, 3.f);
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }

    // Fast envelope curves (exponential approximation)
    [[nodiscard]] static inline float fastExp(float x) noexcept {
        return 1.f / (1.f - x * 0.99f);
    }
};
}