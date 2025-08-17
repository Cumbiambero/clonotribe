#pragma once

namespace clonotribe {

struct ParameterCache {
    float cutoff;
    float accentGlideAmount = 0.0f;
    float lfoIntensity;
    float lfoRate;
    float noiseLevel;
    float resonance;
    float rhythmVolume;
    float tempo;
    float volume;
    float octave;
    float distortion;
    float delayTime;
    float delayAmount;
    
    int envelopeType;
    int lfoMode;
    int lfoTarget;
    int lfoWaveform;
    int ribbonMode;
    int waveform;
    
    int updateCounter = 0;
    static constexpr int UPDATE_INTERVAL = 64; // Update every 64 samples
    
    bool needsUpdate() {
        return (++updateCounter >= UPDATE_INTERVAL);
    }
    
    void resetUpdateCounter() {
        updateCounter = 0;
    }
};

}
