#pragma once
#include <cmath>
#include "fastmath.hpp"

namespace clonotribe {

class Distortion {
public:
    float process(float input, float amount) {
        if (amount <= 0.0f) {
            return input;
        }
        
        float driven = input * (1.0f + amount * DRIVE_SCALE);
        
        driven = FastMath::fastTanh(driven * 1.5f) * 0.7f;
        
        if (driven > THRESHOLD) {
            driven = THRESHOLD + (driven - THRESHOLD) * POSITIVE_CLIPPING_THRESHOLD;
        } else if (driven < -THRESHOLD) {
            driven = -THRESHOLD + (driven + THRESHOLD) * NEGATIVE_CLIPPING_FACTOR;
        }
        
        driven = FastMath::fastTanh(FastMath::fastTanh(driven * 2.5f) * 0.6f * 3.0f) * 0.5f;
        
        float filterCutoff = FILTER_BASE - amount * FILTER_SCALE;
        lowpass += (driven - lowpass) * filterCutoff;
        
        float highFreq = driven - lowpass;
        float output = lowpass + FastMath::fastTanh(highFreq * HIGH_SATURATION_SCALE) * HIGH_MIX * amount;
        
        output = FastMath::fastTanh(output * FINAL_SATURATION_SCALE) * FINAL_GAIN;
        
        return output * (COMPRESSION_BASE / (1.0f + amount * COMPRESSION_SCALE));
    }

private:
    static constexpr float DRIVE_SCALE = 50.0f;
    static constexpr float THRESHOLD = 0.4f;
    static constexpr float POSITIVE_CLIPPING_THRESHOLD = 0.03f;
    static constexpr float NEGATIVE_CLIPPING_FACTOR = 0.05f;
    static constexpr float FILTER_BASE = 0.8f;
    static constexpr float FILTER_SCALE = 0.3f;
    static constexpr float HIGH_SATURATION_SCALE = 8.0f;
    static constexpr float HIGH_MIX = 0.3f;
    static constexpr float FINAL_SATURATION_SCALE = 4.0f;
    static constexpr float FINAL_GAIN = 0.4f;
    static constexpr float COMPRESSION_BASE = 0.8f;
    static constexpr float COMPRESSION_SCALE = 0.1f;

    float lowpass = 0.0f;    
};
}