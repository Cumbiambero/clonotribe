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
        cutoff = std::clamp(fcHz, 1.0f, 100.0f);
        updateCoefficients();
    }
    
    [[nodiscard]] float process(float x) noexcept {
        return processStage1(x);
    }
    
    [[nodiscard]] float processAggressive(float x) noexcept {
        float stage1 = processStage1(x);
        return processStage2(stage1);
    }
    
    [[nodiscard]] float processFinal(float x) noexcept {
        float stage1 = processStage1(x);
        float stage2 = processStage2(stage1);
        return processStage3(stage2);
    }
    
    void reset() noexcept { 
        x1 = x2 = x3 = 0.0f;
        y1 = y2 = y3 = 0.0f;
        dcEstimate = 0.0f;
    }
    
private:
    float sampleRate = 44100.0f;
    float cutoff = 20.0f;
    float R1 = 0.995f, R2 = 0.998f, R3 = 0.9995f;
    float x1 = 0.0f, x2 = 0.0f, x3 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f, y3 = 0.0f;
    float dcEstimate = 0.0f;

    void updateCoefficients() noexcept {
        const float invSampleRate = 1.0f / sampleRate;
        
        const float w1 = FastMath::TWO_PI * cutoff * invSampleRate;
        R1 = (1.0f - w1) / (1.0f + w1);
        R1 = std::clamp(R1, 0.9f, 0.998f);
        
        const float w2 = FastMath::TWO_PI * (cutoff * 0.5f) * invSampleRate;
        R2 = (1.0f - w2) / (1.0f + w2);
        R2 = std::clamp(R2, 0.95f, 0.999f);
        
        const float w3 = FastMath::TWO_PI * (cutoff * 0.25f) * invSampleRate;
        R3 = (1.0f - w3) / (1.0f + w3);
        R3 = std::clamp(R3, 0.98f, 0.9999f);
    }
    
    [[nodiscard]] float processStage1(float x) noexcept {
        if (std::abs(x) < 1e-30f) x = 0.0f;
        x = std::clamp(x, -100.0f, 100.0f);
        
        const float y = x - x1 + R1 * y1;
        x1 = x;
        y1 = (std::abs(y) < 1e-30f) ? 0.0f : y;
        return y1;
    }
    
    [[nodiscard]] float processStage2(float x) noexcept {
        if (std::abs(x) < 1e-30f) x = 0.0f;
        
        const float y = x - x2 + R2 * y2;
        x2 = x;
        y2 = (std::abs(y) < 1e-30f) ? 0.0f : y;
        return y2;
    }
    
    [[nodiscard]] float processStage3(float x) noexcept {
        if (std::abs(x) < 1e-30f) x = 0.0f;
        
        const float dcAlpha = 0.0001f;
        dcEstimate = dcEstimate * (1.0f - dcAlpha) + x * dcAlpha;
        x -= dcEstimate;
        
        const float y = x - x3 + R3 * y3;
        x3 = x;
        y3 = (std::abs(y) < 1e-30f) ? 0.0f : y;
        return y3;
    }
};
}