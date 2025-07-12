#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

// Fast approximations for performance-critical operations
struct FastMath {
    
    // Fast sine approximation using polynomial (much faster than std::sin)
    static inline float fastSin(float x) {
        // Normalize to [-π, π]
        while (x > M_PI) x -= 2.f * M_PI;
        while (x < -M_PI) x += 2.f * M_PI;
        
        // Fast polynomial approximation
        float x2 = x * x;
        return x * (1.f - x2 * (1.f/6.f - x2 * 1.f/120.f));
    }
    
    // Fast cosine approximation (cos(x) = sin(x + π/2))
    static inline float fastCos(float x) {
        return fastSin(x + M_PI * 0.5f);
    }
    
    // Fast tanh approximation
    static inline float fastTanh(float x) {
        // Clamp to reasonable range
        x = clamp(x, -3.f, 3.f);
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }
    
    // Fast envelope curves (exponential approximation)
    static inline float fastExp(float x) {
        return 1.f / (1.f - x * 0.99f); // Approximates exp for envelope shapes
    }
};

}
