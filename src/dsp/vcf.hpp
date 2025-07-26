#pragma once

#include "fastmath.hpp"
#include "noise.hpp"

using namespace clonotribe;

class VCF {
private:
    static constexpr float EPSILON = 1e-10f;

    float xIn1, xIn2;
    float yIn1, yIn2;
    
    float cutoffParam;
    float resonanceParam;
    float sampleRate;
    
    float previousCutoff;
    float previousResonance;
    
    bool active = true;
    
    inline float smoothParameter(float current, float target, float smoothing = 0.01f) {
        return current + (target - current) * smoothing;
    }
    
    inline float calculateCutoffFreq(float param) const {
        param = std::clamp(param, 0.0f, 1.0f);
        
        if (param <= 0.01f) {
            return 20.0f + param * 480.0f;
        }
        
        float freq = 20.0f + param * param * 18000.0f;
        return std::clamp(freq, 20.0f, sampleRate * 0.45f);
    }
    
    void processLowpassFilter(float input, float frequency, float resonance) {
        float omega = 2.0f * FastMath::PI * frequency / sampleRate;
        omega = std::clamp(omega, 0.01f, FastMath::PI * 0.8f);
        float cosOmega = FastMath::fastCos(omega);
        float sinOmega = FastMath::fastSin(omega);

        float q = 0.7f + resonance * resonance * 8.0f; 
        q = std::clamp(q, 0.5f, 10.0f);

        float alpha = sinOmega / (2.0f * q);
        alpha = std::clamp(alpha, EPSILON, 0.5f);

        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        
        if (std::abs(a0) < EPSILON) a0 = EPSILON;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;

        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;

        if (std::abs(a2) >= 0.99f) {
            a2 = std::copysign(0.99f, a2);
        }

        float output = b0 * input + b1 * xIn1 + b2 * xIn2 - a1 * yIn1 - a2 * yIn2;

        output = std::clamp(output, -100.0f, 100.0f);

        xIn2 = xIn1;
        xIn1 = input;
        yIn2 = yIn1;
        yIn1 = output;

        if (!std::isfinite(yIn1) || std::abs(yIn1) > 100.0f) yIn1 = 0.0f;
        if (!std::isfinite(yIn2) || std::abs(yIn2) > 100.0f) yIn2 = 0.0f;
        if (!std::isfinite(xIn1) || std::abs(xIn1) > 100.0f) xIn1 = 0.0f;
        if (!std::isfinite(xIn2) || std::abs(xIn2) > 100.0f) xIn2 = 0.0f;
        
        if (std::abs(yIn1) < 1e-6f) yIn1 = 0.0f;
        if (std::abs(yIn2) < 1e-6f) yIn2 = 0.0f;
        if (std::abs(xIn1) < 1e-6f) xIn1 = 0.0f;
        if (std::abs(xIn2) < 1e-6f) xIn2 = 0.0f;
    }
    
    inline float saturation(float input, float amount = 0.1f) {
        return FastMath::fastTanh(input * (1.0f + amount)) / (1.0f + amount * 0.5f);
    }
    
public:
    VCF() : xIn1(0), xIn2(0), yIn1(0), yIn2(0), 
            cutoffParam(0.5f), resonanceParam(0.0f), 
            sampleRate(44100.0f), previousCutoff(0.5f), previousResonance(0.0f) {}
    
    void setSampleRate(float sampleRate) {
        this->sampleRate = std::clamp(sampleRate, 8000.0f, 192000.0f);
    }
    
    void setCutoff(float cutoffHz) {
        if (cutoffHz <= 20.0f) {
            cutoffParam = 0.0f;
        } else {
            float normalizedFreq = (cutoffHz - 20.0f) / 18000.0f;
            cutoffParam = std::sqrt(std::clamp(normalizedFreq, 0.0f, 1.0f));
        }
        cutoffParam = std::clamp(cutoffParam, 0.0f, 1.0f);
    }
    
    void setResonance(float resonance) {
        resonanceParam = std::clamp(resonance, 0.0f, 1.0f);
    }
    
    float process(float input, float sampleRate, NoiseGenerator& noiseGenerator) {
        if (!active) {
            return input;
        }

        input = std::clamp(input, -10.0f, 10.0f);
        this->sampleRate = std::clamp(sampleRate, 8000.0f, 192000.0f);

        float smoothingFactor = 0.02f;
        previousCutoff = smoothParameter(previousCutoff, cutoffParam, smoothingFactor);
        previousResonance = smoothParameter(previousResonance, resonanceParam, smoothingFactor);

        if (previousCutoff < 0.005f) {
            xIn1 *= 0.9f;
            xIn2 *= 0.9f;
            yIn1 *= 0.9f;
            yIn2 *= 0.9f;
            return 0.0f; 
        }

        float analogNoise = 0.0f;
        if (previousResonance > 0.8f && previousCutoff > 0.1f) {
            analogNoise = noiseGenerator.process() * 0.0002f;
        }

        float cutoffFrequency = calculateCutoffFreq(previousCutoff + analogNoise);
        float saturatedInput = input;

        if (previousResonance > 0.3f) {
            saturatedInput = saturation(input, previousResonance * 0.02f);
        }

        processLowpassFilter(saturatedInput, cutoffFrequency, previousResonance);

        float output = yIn1;
        
        if (previousResonance > 0.3f) {
            output = saturation(output, previousResonance * 0.01f);
        }

        if (previousResonance > 0.5f && previousCutoff > 0.1f) {
            output *= (1.0f + previousResonance * 0.05f);
        }

        if (previousCutoff < 0.1f) {
            float cutoffFade = previousCutoff * 10.0f;
            cutoffFade = cutoffFade * cutoffFade * cutoffFade;
            output *= cutoffFade;
        }

        output = std::clamp(output, -50.0f, 50.0f);

        if (!std::isfinite(output)) {
            output = 0.0f;
        }

        return output;
    }

    void setActive(bool isActive) {
        active = isActive;
    }
    
    void reset() {
        xIn1 = xIn2 = yIn1 = yIn2 = 0.0f;
        previousCutoff = cutoffParam;
        previousResonance = resonanceParam;
    }
};