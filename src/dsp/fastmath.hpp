#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

struct FastMath {
    
    // Fast sine approximation using polynomial
    static inline float fastSin(float x) {
        while (x > M_PI) x -= 2.f * M_PI;
        while (x < -M_PI) x += 2.f * M_PI;
        float x2 = x * x;
        return x * (1.f - x2 * (1.f/6.f - x2 * 1.f/120.f));
    }
    
    static inline float fastCos(float x) {
        return fastSin(x + M_PI * 0.5f);
    }
    
    static inline float fastTanh(float x) {
        x = clamp(x, -3.f, 3.f); // clamp to reasonable range
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }
    
    // Fast envelope curves (exponential approximation)
    static inline float fastExp(float x) {
        return 1.f / (1.f - x * 0.99f);
    }
};
}