

#pragma once
#include <algorithm>
#include <cmath>
#include "../fastmath.hpp"

namespace clonotribe {

class LadderFilter {
public:
    LadderFilter() = default;

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
        float f = TWO * FastMath::fastSin(FastMath::PI * cutoff * invSampleRate);
        f = std::clamp(f, 0.f, 0.99f);
        float x = input - res * y4;
        y1 += f * (FastMath::fastTanh(x - y1));
        y2 += f * (FastMath::fastTanh(y1 - y2));
        y3 += f * (FastMath::fastTanh(y2 - y3));
        y4 += f * (FastMath::fastTanh(y3 - y4));
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