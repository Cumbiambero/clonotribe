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
    bool active = true;
    bool sampleAndHold = false;
    
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
            sampleHoldValue = (random::uniform() - 0.5f) * 2.f;
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
        
        if (sampleAndHold) {
            if (phase < lastPhase) {
                sampleHoldValue = (random::uniform() - 0.5f) * 2.f;
            }
            output = sampleHoldValue;
        } else {
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
                case 3:
                    if (phase < lastPhase) {
                        sampleHoldValue = (random::uniform() - 0.5f) * 2.f;
                    }
                    output = sampleHoldValue;
            }
        }
        
        lastPhase = phase;
        return output;
    }
};
}