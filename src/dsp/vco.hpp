#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

using namespace rack;

namespace clonotribe {

struct VCO {
    float phase = 0.f;
    float freq = 440.f;
    float pulseWidth = 0.5f;
    float lastSaw = 0.f;
    float lastPulse = 0.f;
    bool active = true;
    
    dsp::RCFilter antiAlias;
    
    VCO() {
        antiAlias.setCutoff(8000.f / 48000.f); // 8kHz cutoff
    }
    
    void setPitch(float pitch) {
        freq = dsp::FREQ_C4 * std::pow(2.f, pitch);
        active = (freq > 1.f);
    }
    
    void setPulseWidth(float pw) {
        pulseWidth = clamp(pw, 0.01f, 0.99f);
    }
    
    float processSaw(float sampleTime) {
        if (!active) return lastSaw;
        
        phase += freq * sampleTime;
        if (phase >= 1.f) phase -= 1.f;
        
        float saw = 2.f * phase - 1.f;
        saw = saw + 0.1f * saw * saw * saw; // add harmonics
        
        antiAlias.process(saw);
        lastSaw = saw;
        return saw;
    }
    
    float processPulse(float sampleTime) {
        if (!active) return lastPulse;
        
        phase += freq * sampleTime;
        if (phase >= 1.f) phase -= 1.f;
        
        float pulse = (phase < pulseWidth) ? 1.f : -1.f;
        
        float transition = 0.01f;
        if (phase > pulseWidth - transition && phase < pulseWidth + transition) {
            float t = (phase - (pulseWidth - transition)) / (2.f * transition);
            pulse = 1.f - 2.f * t;
        }
        
        antiAlias.process(pulse);
        lastPulse = pulse;
        return pulse;
    }
    
    float processTriangle(float sampleTime) {
        if (!active) return 0.f;
        
        phase += freq * sampleTime;
        if (phase >= 1.f) phase -= 1.f;
        
        float triangle;
        if (phase < 0.5f) {
            triangle = 4.f * phase - 1.f; 
        } else {
            triangle = 3.f - 4.f * phase; 
        }
        
        triangle = triangle + 0.05f * triangle * triangle * triangle;
        
        antiAlias.process(triangle);
        return triangle;
    }
    
    float processSquare(float sampleTime) {
        if (!active) return 0.f;
        
        phase += freq * sampleTime;
        if (phase >= 1.f) phase -= 1.f;
        
        float square = (phase < 0.5f) ? 1.f : -1.f;
        
        float transition = 0.005f;
        if (phase > 0.5f - transition && phase < 0.5f + transition) {
            float t = (phase - (0.5f - transition)) / (2.f * transition);
            square = 1.f - 2.f * t;
        } else if (phase < transition) {
            float t = phase / transition;
            square = -1.f + 2.f * t;
        } else if (phase > 1.f - transition) {
            float t = (phase - (1.f - transition)) / transition;
            square = 1.f - 2.f * t;
        }
        
        antiAlias.process(square);
        return square;
    }
};
}