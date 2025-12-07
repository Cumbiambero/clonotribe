#pragma once
#include "../fastmath.hpp"
#include "../noise.hpp"
#include <algorithm>
#include <cmath>

namespace clonotribe {

class MS20Filter {
public:
    MS20Filter() = default;

    void setSampleRate(float sr) noexcept {
        sampleRate = std::max(8000.f, sr);
        invSampleRate = ONE / sampleRate;
        oversampleFactor = (sampleRate > 88200.f) ? 1 : 2;
        noiseGen.setSeed(static_cast<uint32_t>(sr));
    }

    void setCutoff(float param) noexcept {
        cutoffParam = std::clamp(param, 0.f, ONE);
        updateCoefficients();
    }

    void setResonance(float param) noexcept {
        resonanceParam = std::clamp(param, 0.f, ONE);
        updateCoefficients();
    }

    void setActive(bool isActive) noexcept {
        active = isActive;
        if (!active) reset();
    }

    [[nodiscard]] float process(float input) noexcept {
        if (!active) {
            return ZERO;
        }

        input = flushDenormals(input);

        if (!std::isfinite(input)) {
            reset();
            return ZERO;
        }

        float output = ZERO;
        
        for (int os = 0; os < oversampleFactor; ++os) {
            float osInput = (os == 0) ? input : ZERO;
            output = processSample(osInput);
        }
        
        return output;
    }

    void reset() noexcept {
        s1 = s2 = ZERO;
        ic1eq = ic2eq = ZERO;
        oscPhase = ZERO;
        lastOutput = ZERO;
        dcInput = dcOutput = ZERO;
    }

private:
    float s1 = ZERO, s2 = ZERO;
    float ic1eq = ZERO, ic2eq = ZERO;
    float dcInput = ZERO, dcOutput = ZERO;
    float cutoffParam = HALF;
    float resonanceParam = ZERO;
    float sampleRate = 44100.f;
    float invSampleRate = ONE / 44100.f;
    float g = ZERO;
    float k = ZERO;
    float oscPhase = ZERO;
    float lastOutput = ZERO;
    
    int oversampleFactor = 2;
    bool active = true;
    
    NoiseGenerator noiseGen;

    void updateCoefficients() noexcept {
        float cutoffHz = calculateCutoffFrequency(cutoffParam);
        
        float effectiveSampleRate = sampleRate * oversampleFactor;
        float wd = TWO * FastMath::PI * cutoffHz;
        float wa = TWO * effectiveSampleRate * std::tan(wd / (TWO * effectiveSampleRate));
        g = wa / (TWO * effectiveSampleRate);
        g = std::clamp(g, 0.0001f, 0.99f);
        k = calculateResonance(resonanceParam);
    }

    [[nodiscard]] float calculateCutoffFrequency(float param) const noexcept {
        param = std::clamp(param, 0.0f, ONE);
        float minFreq = 20.0f;
        float maxFreq = std::min(18000.0f, sampleRate * 0.45f);
        float normalizedFreq = std::exp(param * std::log(maxFreq / minFreq));
        return minFreq * normalizedFreq;
    }

    [[nodiscard]] float calculateResonance(float param) const noexcept {
        param = std::clamp(param, 0.f, ONE);
        float sqrtParam = std::sqrt(param);
        float baseK = sqrtParam * 1.2f + param * param * 0.8f;
        
        if (param > 0.6f) {
            float extra = (param - 0.6f) * 2.5f;
            baseK += extra * extra * 0.8f;
        }
        return std::min(baseK, 2.8f);
    }

    [[nodiscard]] float processSample(float input) noexcept {
        float noise = noiseGen.generateWhiteNoise() * 1e-8f;
        input += noise;
        float drive = ONE + resonanceParam * 0.5f;
        float drivenInput = input * drive;
        drivenInput = saturateSoft(drivenInput);
        
        float g2 = g * g;
        float d = ONE / (ONE + g * k + g2);
        float hp = (drivenInput - k * s1 - s2) * d;
        float bp = g * hp + s1;
        float lp = g * bp + s2;
        s1 = g * hp + bp;
        s2 = g * bp + lp;
        
        s1 = flushDenormals(saturateOTA(s1));
        s2 = flushDenormals(saturateOTA(s2));
        
        float output = lp;
        float gainComp = 3.0f / (ONE + k * 0.3f);
        output *= gainComp;
        
        if (resonanceParam > 0.75f) {
            float oscStrength = (resonanceParam - 0.75f) * 4.0f;
            
            float cutoffHz = calculateCutoffFrequency(cutoffParam);
            float phaseInc = cutoffHz / (sampleRate * oversampleFactor);
            
            oscPhase += phaseInc;
            if (oscPhase >= ONE) oscPhase -= ONE;
            
            float oscSig = FastMath::fastSin(oscPhase * TWO * FastMath::PI);
            
            float oscLevel = oscStrength * oscStrength * 0.35f;
            output = output * (ONE - oscStrength * 0.25f) + oscSig * oscLevel;
            
            if (resonanceParam > 0.9f) {
                float extra = (resonanceParam - 0.9f) * 10.0f;
                output += oscSig * extra * 0.2f;
            }
        }
        
        output = saturateSoft(output);
        
        float dcCoeff = 0.9997f;
        float dcIn = output;
        output = dcCoeff * (dcOutput + output - dcInput);
        dcInput = dcIn;
        dcOutput = output;
        
        return flushDenormals(output);
    }

    [[nodiscard]] float saturateOTA(float x) const noexcept {
        x = std::clamp(x, -3.f, 3.f);
        float absX = std::abs(x);
        float denom = ONE + absX * 0.6f + x * x * 0.1f;
        
        if (x < ZERO) {
            denom += absX * 0.1f;
        }
        
        return x / denom;
    }

    [[nodiscard]] float saturateSoft(float x) const noexcept {
        x = std::clamp(x, -4.f, 4.f);
        return x / (ONE + std::abs(x) * 0.15f);
    }

    [[nodiscard]] float saturateMS20(float x) const noexcept {
        x = std::clamp(x, -5.f, 5.f);
        if (x > ZERO) {
            return x / (ONE + std::abs(x) * 0.3f);
        } else {
            return x / (ONE + std::abs(x) * 0.5f);
        }
    }

    [[nodiscard]] float flushDenormals(float x) const noexcept {
        if (std::abs(x) < 1e-20f) {
            return ZERO;
        }
        return x;
    }
};
}