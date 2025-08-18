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
        cutoffParam = std::clamp(param, 0.f, 1.f);
    }
    void setResonance(float param) noexcept {
        resonanceParam = std::clamp(param, 0.f, 1.f);
    }
    void setActive(bool isActive) noexcept {
        active = isActive;
        if (!active) reset();
    }
    float process(float input) noexcept {
        if (!active) return 0.f;
        float cutoff = 20.f * std::exp(7.0f * cutoffParam);
        float res = resonanceParam * 4.0f;
        float f = cutoff * invSampleRate * 1.16f;
        float fb = res * (1.0f - 0.15f * f * f);
        float in = input - fb * y4;
        in = FastMath::fastTanh(in);
        y1 = FastMath::fastTanh(in * f + FastMath::fastTanh(y1) * (1.0f - f));
        y2 = FastMath::fastTanh(y1 * f + FastMath::fastTanh(y2) * (1.0f - f));
        y3 = FastMath::fastTanh(y2 * f + FastMath::fastTanh(y3) * (1.0f - f));
        y4 = FastMath::fastTanh(y3 * f + FastMath::fastTanh(y4) * (1.0f - f));
        return y4;
    }
    void reset() noexcept {
        y1 = y2 = y3 = y4 = 0.f;
    }
private:
    float y1 = 0.f, y2 = 0.f, y3 = 0.f, y4 = 0.f;
    float cutoffParam = 0.5f;
    float resonanceParam = 0.f;
    float sampleRate = 44100.f;
    float invSampleRate = FastMath::fastInverse(44100.f);
    bool active = true;
};

}
