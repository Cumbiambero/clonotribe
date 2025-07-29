#pragma once

#include "fastmath.hpp"
#include "noise.hpp"

namespace clonotribe {

class VCF {
private:
    // MS-20 filter specifications
    static constexpr float MS20_MIN_FREQ = 20.0f;
    static constexpr float MS20_MAX_FREQ = 20000.0f;
    static constexpr float MS20_Q_GAIN = 12.0f;
    static constexpr float MS20_THERMAL_COEFF = 0.02f;
    static constexpr float MS20_DRIFT_RANGE = 0.01f;
    static constexpr float PROBLEM_RANGE_START = 0.62f;
    static constexpr float PROBLEM_RANGE_END = 0.70f;
    static constexpr float PROBLEM_RANGE_CENTER = 0.66f;
    static constexpr float CUTOFF_MUTE_THRESHOLD = 0.28f;

    // State variables
    float x1 = 0.f, x2 = 0.f;
    float y1 = 0.f, y2 = 0.f;
    float cutoffParam = 0.5f;
    float resonanceParam = 0.f;
    float sampleRate = 44100.f;
    float invSampleRate = 1.f/44100.f;
    float thermalState = 0.f;
    bool active = true;

    NoiseGenerator componentDrift;

    // Frequency calculation with smooth transitions
    [[nodiscard]] inline float calculateCutoffHz(float param) const noexcept {
        param = std::clamp(param, 0.f, 1.f);
        // Piecewise response curve
        if (param < 0.78f) {
            return 20.0f + std::pow(param * 1.28f, 3.0f) * 18000.0f;
        }
        return 20.0f + std::exp(6.5f * param) * 15.0f;
    }

    // Resonance calculation with problem range adjustment
    [[nodiscard]] inline float calculateResonance(float param) const noexcept {
        param = std::clamp(param, 0.f, 1.f);
        float q = 0.5f + param * 2.5f + std::pow(param, 3.0f) * 10.0f;
        return std::clamp(q, 0.5f, MS20_Q_GAIN);
    }

    // Saturation with problem range adjustment
    [[nodiscard]] inline float ms20Saturation(float x, bool inProblemRange) const noexcept {
        x = std::clamp(x, -5.f, 5.f);
        if (inProblemRange) {
            return FastMath::fastTanh(x * 0.9f) * 1.1f; // Softer saturation
        }
        if (x > 0) return FastMath::fastTanh(x * 1.05f) * 0.95f;
        return FastMath::fastTanh(x * 0.95f) * 1.05f;
    }

    void processFilter(float input, float cutoffHz, float resonance) noexcept {
        const bool inProblemRange = (cutoffParam > PROBLEM_RANGE_START) && 
                                  (cutoffParam < PROBLEM_RANGE_END);

        // Problem range specific processing
        if (inProblemRange) {
            // Apply bell-curve damping centered at 0.66
            float damping = 1.f - std::abs(cutoffParam - PROBLEM_RANGE_CENTER) * 5.f;
            resonance *= std::clamp(damping, 0.8f, 1.f);
            
            // Gentle lowpass on cutoff modulation
            cutoffHz = cutoffHz * 0.99f + calculateCutoffHz(cutoffParam) * 0.01f;
        }

        // Thermal and drift modeling
        thermalState += (std::abs(input) - thermalState) * 0.0005f;
        float driftedCutoff = cutoffHz * (1.f + thermalState * MS20_THERMAL_COEFF);
        driftedCutoff += componentDrift.process() * MS20_DRIFT_RANGE * cutoffHz;

        // Filter coefficients
        const float omega = 2.f * FastMath::PI * std::clamp(driftedCutoff, MS20_MIN_FREQ, sampleRate*0.45f) * invSampleRate;
        const float sinOmega = FastMath::fastSin(std::clamp(omega, 0.01f, FastMath::PI*0.8f));
        const float cosOmega = FastMath::fastCos(omega);

        float alpha = sinOmega / (2.f * resonance);
        alpha = std::clamp(alpha, 0.0001f, 0.5f) * 0.999f;

        const float b0 = (1.f - cosOmega) * 0.5f;
        const float b1 = 1.f - cosOmega;
        const float b2 = b0;
        const float a0 = 1.f + alpha;
        const float a1 = -2.f * cosOmega;
        const float a2 = 1.f - alpha;

        // Process sample
        float output = (b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;

        // State update with stability checks
        x2 = x1; x1 = input + 1e-20f; // Anti-denormal
        y2 = y1; y1 = output + 1e-20f;
        if (!std::isfinite(y1)) reset();
    }

public:
    VCF() = default;

    void reset() noexcept {
        x1 = x2 = y1 = y2 = 0.f;
        thermalState = 0.f;
    }

    void setSampleRate(float sr) noexcept {
        sampleRate = std::clamp(sr, 8000.f, 192000.f);
        invSampleRate = 1.f / sampleRate;
        componentDrift.setSeed(static_cast<uint32_t>(sampleRate));
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


    [[nodiscard]] float process(float input) noexcept {
        if (!active) return input;

        if (cutoffParam < CUTOFF_MUTE_THRESHOLD) {
            reset();
            return 0.f;
        }

        const bool inProblemRange = (cutoffParam > PROBLEM_RANGE_START) && 
                                  (cutoffParam < PROBLEM_RANGE_END);

        // Input processing
        input = ms20Saturation(input, inProblemRange);
        
        // Noise injection (reduced in problem range)
        if (resonanceParam > 0.7f) {
            float noiseAmount = inProblemRange ? 0.00006f : 0.0002f;
            input += componentDrift.process() * noiseAmount * resonanceParam;
        }

        // Filter processing
        processFilter(input, calculateCutoffHz(cutoffParam), calculateResonance(resonanceParam));

        // Output processing
        float output = y1;
        if (inProblemRange) {
            output = std::clamp(output, -2.f, 2.f);
        }
        output = ms20Saturation(output, inProblemRange) * (1.f + resonanceParam * 0.03f);
        
        return std::clamp(output, -5.f, 5.f);
    }

    
   
};
}