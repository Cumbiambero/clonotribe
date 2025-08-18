#pragma once

#include "fastmath.hpp"

namespace clonotribe {

class DcBlocker final {
public:
    void setSampleRate(float sampleRate) noexcept {
        this->sampleRate = (sampleRate > 100.0f) ? sampleRate : 44100.0f;
        updateCoeff();
    }
    void setCutoff(float fcHz) noexcept {
        cutoff = (fcHz > 0.1f) ? fcHz : 20.0f;
        updateCoeff();
    }
    float process(float x) noexcept {
        const float y = x - x1 + R * y1;
        x1 = x;
        y1 = (y > -1e-20f && y < 1e-20f) ? 0.0f : y;
        return y1;
        return x;
    }
    void reset() noexcept { x1 = y1 = 0.0f; }
private:
    void updateCoeff() noexcept {
    const float x = FastMath::TWO_PI * cutoff * FastMath::fastInverse(sampleRate);
        R = (1.0f - x) / (1.0f + x);
        if (R < 0.0f) R = 0.0f;
        if (R > 0.9999f) R = 0.9999f;
    }
    float sampleRate = 44100.0f;
    float cutoff = 20.0f;
    float R = 0.995f;
    float x1 = 0.0f;
    float y1 = 0.0f;
};
}