#pragma once

#include "fastmath.hpp"
#include <algorithm>
#include <cmath>

using namespace clonotribe;

class VCF {
private:
    static constexpr float EPSILON = 1e-10f;

    float z1, z2;
    float cutoffParam;
    float resonanceParam;
    float sampleRate;
    float prevCutoff;
    float prevResonance;

    bool active{true};    
    
    inline float smoothParameter(float current, float target, float smoothing = 0.005f) {
        return current + (target - current) * smoothing;
    }
    
    inline float calculateCutoffFreq(float param) const {
        param = std::clamp(param, 0.0f, 1.0f);
        
        if (param <= 0.02f) {
            return 0.5f + param * 2.0f;
        }
        
        float freq;
        if (param <= 0.1f) {
            float t = param / 0.1f;
            freq = 0.5f + t * t * t * 19.5f;
        } else if (param <= 0.4f) {
            float t = (param - 0.1f) / 0.3f;
            freq = 20.0f + t * t * 180.0f;
        } else if (param <= 0.7f) {
            float t = (param - 0.4f) / 0.3f;
            freq = 200.0f + t * 1800.0f; 
        } else {
            float t = (param - 0.7f) / 0.3f;
            freq = 2000.0f + t * t * 6000.0f;
        }
        
        return std::clamp(freq, 0.1f, sampleRate * 0.45f);
    }
    
    void processMS20Filter(float input, float frequency, float resonance) {
        float omega = 2.0f * FastMath::PI * frequency / sampleRate;
        omega = std::clamp(omega, 0.0001f, FastMath::PI * 0.95f);
        
        float cosOmega = std::cos(omega);
        float sinOmega = std::sin(omega);
        
        float q;
        if (resonance <= 0.01f) {
            q = 0.5f;
        } else {
            q = 0.7f + resonance * resonance * 25.0f;
        }
        q = std::clamp(q, 0.1f, 30.0f);
        
        float alpha = sinOmega / (2.0f * q);
        alpha = std::clamp(alpha, EPSILON, 0.95f);
        
        float b0 = (1.0f - cosOmega) / 2.0f;
        float b1 = 1.0f - cosOmega;
        float b2 = (1.0f - cosOmega) / 2.0f;
        float a0 = 1.0f + alpha;
        float a1 = -2.0f * cosOmega;
        float a2 = 1.0f - alpha;
        
        b0 /= a0;
        b1 /= a0;
        b2 /= a0;
        a1 /= a0;
        a2 /= a0;
        
        float output = b0 * input + b1 * z1 + b2 * z2 - a1 * z1 - a2 * z2;
        
        z2 = z1;
        z1 = input;
        
        if (std::abs(output) < EPSILON) output = 0.0f;
        if (std::abs(z1) < EPSILON) z1 = 0.0f;
        if (std::abs(z2) < EPSILON) z2 = 0.0f;
        
        this->z1 = output;
    }
    
    inline float ms20Distortion(float input) {
        return FastMath::fastTanh(input * 0.9f) * 1.1f;
    }
    
public:
    VCF() : z1(0), z2(0), cutoffParam(0.5f), resonanceParam(0.0f), 
            sampleRate(44100.0f), prevCutoff(0.5f), prevResonance(0.0f) {}
    
    void setSampleRate(float sampleRate) {
        this->sampleRate = std::clamp(sampleRate, 8000.0f, 192000.0f);
    }
    
    void setCutoff(float cutoffHz) {
        if (cutoffHz <= 1.0f) {
            cutoffParam = cutoffHz / 50.0f;
        } else if (cutoffHz <= 20.0f) {
            cutoffParam = 0.02f + (cutoffHz - 1.0f) / 190.0f * 0.08f;
        } else if (cutoffHz <= 200.0f) {
            cutoffParam = 0.1f + (cutoffHz - 20.0f) / 180.0f * 0.3f;
        } else if (cutoffHz <= 2000.0f) {
            cutoffParam = 0.4f + (cutoffHz - 200.0f) / 1800.0f * 0.3f;
        } else {
            cutoffParam = 0.7f + (cutoffHz - 2000.0f) / 6000.0f * 0.3f;
        }
        cutoffParam = std::clamp(cutoffParam, 0.0f, 1.0f);
    }
    
    void setResonance(float resonance) {
        resonanceParam = std::clamp(resonance, 0.0f, 1.0f);
    }
    
    float process(float input, float sampleRate) {
        if (!active) {
            return input;
        }
        
        input = std::clamp(input, -10.0f, 10.0f);
        this->sampleRate = std::clamp(sampleRate, 8000.0f, 192000.0f);
        
        prevCutoff = smoothParameter(prevCutoff, cutoffParam, 0.002f);
        prevResonance = smoothParameter(prevResonance, resonanceParam, 0.002f);
        
        float cutoffFrequency = calculateCutoffFreq(prevCutoff);
        
        float lowFreqAttenuation = 1.0f;
        if (prevCutoff <= 0.05f) {
            lowFreqAttenuation = prevCutoff * 20.0f;
            lowFreqAttenuation = std::clamp(lowFreqAttenuation, 0.001f, 1.0f);
        }
        
        float attenuatedInput = input * lowFreqAttenuation;
        
        float processedInput = ms20Distortion(attenuatedInput);
        
        processMS20Filter(processedInput, cutoffFrequency, prevResonance);
        
        float output = ms20Distortion(z1 * 0.8f);
        
        return output;
    }

    void setActive(bool isActive) {
        active = isActive;
    }
    
    float getCurrentCutoffFreq() const {
        return calculateCutoffFreq(cutoffParam);
    }
    
    void reset() {
        z1 = z2 = 0.0f;
        prevCutoff = cutoffParam;
        prevResonance = resonanceParam;
    }
    
    bool isStable() const {
        return std::isfinite(z1) && std::isfinite(z2) && 
               std::abs(z1) < 100.0f && std::abs(z2) < 100.0f;
    }
};