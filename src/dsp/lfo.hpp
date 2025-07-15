#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

using namespace rack;

namespace clonotribe {

struct LFO {
    float phase = 0.f;
    float freq = 1.f;
    bool oneShot = false;
    bool triggered = false;
    bool active = true; // For conditional processing
    bool sampleAndHold = false; // New: Sample & Hold mode
    
    // Sample & Hold variables
    float sampleHoldValue = 0.f;
    float lastPhase = 0.f;
    
    void setRate(float rate) {
        freq = clamp(rate, 0.01f, 20.f);
        active = (freq > 0.001f); // Skip processing for very low frequencies
    }
    
    void setOneShot(bool os) {
        oneShot = os;
        if (os && !triggered) {
            phase = 0.f;
        }
    }
    
    void setSampleAndHold(bool sh) {
        sampleAndHold = sh;
        if (sh) {
            // Initialize sample hold value
            sampleHoldValue = (rand() / (float)RAND_MAX) * 2.f - 1.f; // Random initial value
        }
    }
    
    void trigger() {
        if (oneShot) {
            phase = 0.f;
            triggered = true;
        }
    }
    
    float process(float sampleTime, int waveform = 0) {
        if (!active || (oneShot && !triggered)) {
            return 0.f;
        }
        
        phase += freq * sampleTime;
        
        if (oneShot && phase >= 0.5f) {
            // In one-shot mode, stop after half cycle
            triggered = false;
            return 0.f;
        }
        
        if (phase >= 1.f) {
            phase -= 1.f;
            if (oneShot) {
                triggered = false;
            }
        }
        
        float output = 0.f;
        
        // Handle Sample & Hold mode override
        if (sampleAndHold) {
            // In Sample & Hold mode, generate new random value at regular intervals
            if (phase < lastPhase) { // Phase wrapped around
                sampleHoldValue = (random::uniform() - 0.5f) * 2.f;
            }
            output = sampleHoldValue;
        } else {
            // Normal waveform processing
            switch (waveform) {
                case 0: // Square
                    output = (phase < 0.5f) ? 1.f : -1.f;
                    break;
                case 1: // Triangle
                    if (phase < 0.5f) {
                        output = 4.f * phase - 1.f;
                    } else {
                        output = 3.f - 4.f * phase;
                    }
                    break;
                case 2: // Sawtooth
                    output = 2.f * phase - 1.f;
                    break;
                case 3: // Sample & Hold (new in firmware 2.1)
                    // Generate new random value when phase wraps
                    if (phase < lastPhase) {
                        sampleHoldValue = (random::uniform() - 0.5f) * 2.f;
                    }
                    output = sampleHoldValue;
                    break;
                default: // Sine using fast approximation
                    output = FastMath::fastSin(2.f * M_PI * phase);
                    break;
            }
        }
        
        lastPhase = phase;
        return output;
    }
};

}
