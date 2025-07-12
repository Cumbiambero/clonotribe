#pragma once
#include <rack.hpp>
#include "fastmath.hpp"

using namespace rack;

namespace clonotribe {

struct KickDrum {
    float phase = 0.f;
    float envelope = 0.f;
    bool triggered = false;
    
    void trigger() {
        phase = 0.f;
        envelope = 1.f;
        triggered = true;
    }
    
    float process(float sampleTime) {
        if (!triggered) return 0.f; // Early exit for performance
        
        float pitchEnv = envelope * envelope * envelope * 45.f + 35.f;
        phase += pitchEnv * sampleTime * 2.f * M_PI;
        if (phase >= 2.f * M_PI) phase -= 2.f * M_PI;
        
        // Use fast sine approximation
        float sine = FastMath::fastSin(phase);
        
        float subPhase = phase * 0.5f;
        float subSine = FastMath::fastSin(subPhase) * 0.4f;
        
        float click = (envelope > 0.9f) ? (envelope - 0.9f) * 10.f : 0.f;
        
        envelope -= sampleTime * 2.5f; 
        if (envelope <= 0.f) {
            envelope = 0.f;
            triggered = false;
        }
        
        return (sine + subSine + click * 0.2f) * envelope * envelope * 4.0f; // Balanced volume with other drums
    }
};

}
