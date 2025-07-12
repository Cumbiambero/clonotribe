#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

using namespace rack;

namespace clonotribe {

struct MS20Filter {
    float cutoff = 1000.f;
    float resonance = 0.f;
    float x1 = 0.f, x2 = 0.f;
    float y1 = 0.f, y2 = 0.f;
    float z1 = 0.f;
    bool active = true; // For conditional processing
    
    void setActive(bool active) {
        this->active = active;
    }
    
    void setCutoff(float freq) {
        cutoff = clamp(freq, 20.f, 20000.f);
    }
    
    void setResonance(float res) {
        resonance = clamp(res, 0.f, 4.f);
    }
    
    float process(float input, float sampleRate) {
        if (!active) return input; // Skip processing if inactive
        
        float omega = 2.f * M_PI * cutoff / sampleRate;
        float cos_omega = FastMath::fastCos(omega);
        float sin_omega = FastMath::fastSin(omega);
        
        float feedback = resonance * 0.9f;
        float nonlinearFeedback = FastMath::fastTanh(feedback * z1) * 0.7f;
        
        input -= nonlinearFeedback;
        
        // Sallen-Key topology approximation
        float alpha = sin_omega / (2.f * (1.f + feedback * 0.1f));
        
        float b0 = (1.f - cos_omega) / 2.f;
        float b1 = 1.f - cos_omega;
        float b2 = (1.f - cos_omega) / 2.f;
        float a0 = 1.f + alpha;
        float a1 = -2.f * cos_omega;
        float a2 = 1.f - alpha;
        
        float output = (b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2) / a0;
        
        output = FastMath::fastTanh(output * 1.2f) * 0.8f;
        
        x2 = x1;
        x1 = input;
        y2 = y1;
        y1 = output;
        z1 = output; // Store for feedback
        
        return output;
    }
};

}
