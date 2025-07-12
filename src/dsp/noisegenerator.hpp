#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

struct NoiseGenerator {
    uint32_t state = 12345;
    
    void setSeed(uint32_t seed) {
        state = seed;
    }
    
    float process() {
        state = state * 1664525 + 1013904223;
        return (float)((uint32_t)state % 65536) / 32768.f - 1.f;
    }
    
    void processStereo(float& left, float& right) {
        left = process();
        right = process();
    }
    
    void reset() {
        state = 12345;
    }
};

}
