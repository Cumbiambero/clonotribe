#pragma once
#include <algorithm>
#include <cmath>
#include "../fastmath.hpp"

namespace clonotribe {

class MoogFilter {
public:
    MoogFilter() = default;
    void setSampleRate(float sr) noexcept {
        sampleRate = std::max(8000.f, sr);
        invSampleRate = FastMath::fastInverse(sampleRate);
    }

    void setCutoff(float param) noexcept {
        cutoffParam = std::clamp(param, 0.f, ONE);
    }

    void setResonance(float param) noexcept {
        resonanceParam = std::clamp(param, 0.f, ONE);
    }

    void setActive(bool isActive) noexcept {
        active = isActive;
        if (!active) reset();
    }

    [[nodiscard]] float process(float input) noexcept {
        if (!active) return ZERO;
        float cutoff = 20.f * std::exp(7.0f * cutoffParam);
        float res = resonanceParam * 4.0f;
        float f = cutoff * invSampleRate * 1.16f;
        float fb = res * (ONE - 0.15f * f * f);
        float in = input - fb * y4;
        in = FastMath::fastTanh(in);
        y1 = FastMath::fastTanh(in * f + FastMath::fastTanh(y1) * (ONE - f));
        y2 = FastMath::fastTanh(y1 * f + FastMath::fastTanh(y2) * (ONE - f));
        y3 = FastMath::fastTanh(y2 * f + FastMath::fastTanh(y3) * (ONE - f));
        y4 = FastMath::fastTanh(y3 * f + FastMath::fastTanh(y4) * (ONE - f));
        return y4;
    }

    void reset() noexcept {
        y1 = y2 = y3 = y4 = ZERO;
    }

private:
    float y1 = 0.f, y2 = 0.f, y3 = 0.f, y4 = ZERO;
    float cutoffParam = HALF;
    float resonanceParam = ZERO;
    float sampleRate = 44100.f;
    float invSampleRate = FastMath::fastInverse(44100.f);
    bool active = true;
};
}