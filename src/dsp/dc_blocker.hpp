#pragma once
#include "fastmath.hpp"

namespace clonotribe {

class DcBlocker final {
public:
    DcBlocker() noexcept {
        reset();
        updateCoefficients();
    }
    
    void setSampleRate(float sampleRate) noexcept {
        this->sampleRate = (sampleRate > 100.0f) ? sampleRate : 44100.0f;
        updateCoefficients();
    }
    
    void setCutoff(float fcHz) noexcept {
        cutoff = (fcHz > 0.1f) ? fcHz : 20.0f;
        updateCoefficients();
    }
    
    [[nodiscard]] float process(float x) noexcept {
        const float y = x - x1 + R * y1;
        x1 = x;
        y1 = (std::abs(y) < 1e-8f) ? 0.0f : y;
        return y1;
    }
    
    void reset() noexcept { 
        x1 = 0.0f;
        y1 = 0.0f; 
    }
    
private:
    float sampleRate = 44100.0f;
    float cutoff = 30.0f;
    float R = 0.995f;
    float x1 = 0.0f;
    float y1 = 0.0f;

    void updateCoefficients() noexcept {
        const float w = FastMath::TWO_PI * cutoff * (1.0f / sampleRate);
        R = (1.0f - w) / (1.0f + w);
        R = std::clamp(R, 0.0f, 0.99f);
    }
};
}