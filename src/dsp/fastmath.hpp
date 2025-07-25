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

    [[nodiscard]] static inline float fastSin(float x) noexcept {        
        while (x > PI) x -= 2.f * PI;
        while (x < -PI) x += 2.f * PI;
        float x2 = x * x;
        return x * (1.0f - x2 * (1.0f/6.0f - x2 * 1.0f/120.0f));
    }

    [[nodiscard]] static inline float fastCos(float x) noexcept {
        return fastSin(x + PI * 0.5f);
    }

    [[nodiscard]] static inline float fastTanh(float x) noexcept {
        x = std::clamp(x, -3.0f, 3.0f);
        float x2 = x * x;
        return x * (27.0f + x2) / (27.0f + 9.0f * x2);
    }

    [[nodiscard]] static inline float fastExp(float x) noexcept {
        return 1.0f / (1.0f - x * 0.99f);
    }

    [[nodiscard]] static inline float fastExp2(float x) noexcept {
        float x2 = x * x;
        float x3 = x2 * x;
        return 1.0f + x * 0.6931f + x2 * 0.2402f + x3 * 0.0555f;
    }
};
}