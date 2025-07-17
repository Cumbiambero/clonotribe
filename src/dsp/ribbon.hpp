#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

struct Ribbon {
    float position = 0.f;
    bool touching = false;
    int mode = 1;
    float octave = 0.f;
    
    void setPosition(float pos) {
        position = clamp(pos, 0.f, 1.f);
    }
    
    void setTouching(bool touch) {
        touching = touch;
    }
    
    void setMode(int m) {
        mode = clamp(m, 0, 2);
    }
    
    void setOctave(float oct) {
        octave = oct;
    }
    
    float getCV() {
        if (!touching) return 0.f;
        
        float cv = 0.f;
        
        switch (mode) {
            case 0: // KEY - chromatic steps, respects octave (Ccromatic steps + octave)
                {
                    int step = (int)(position * 12.f);
                    cv = (step / 12.f) + octave;
                }
                break;
            case 1: // NARROW - continuous, respects octave (±0.5 octave + octave setting)
                cv = (position - 0.5f) * 1.f + octave; 
                break;
                
            case 2: // WIDE - 6x range, ignores octave  (±3 octaves)
                cv = (position - 0.5f) * 6.f;
        }
        
        return cv;
    }
    
    float getGate() {
        return touching ? 5.f : 0.f;
    }
    
    // Get gate time modulation (0.1 to 1.0) based on ribbon position (default gate time when not touching)
    float getGateTimeMod() {
        if (!touching) return 0.5f;
        return clamp(position * 0.9f + 0.1f, 0.1f, 1.0f);
    }
    
    // Get volume automation (-1.0 to +1.0) for synth part volume (no volume change when not touching)
    float getVolumeAutomation() {
        if (!touching) return 0.f; 
        return (position - 0.5f) * 2.f; // -1.0 to +1.0
    }
    
    // Get drum roll intensity (0.0 to 1.0) for drum parts
    float getDrumRollIntensity() {
        if (!touching) return 0.f;
        return position;
    }
};
}