#pragma once

namespace clonotribe {

struct ParameterCache {
    // Cache parameters that don't change often
    float cutoff;
    float lfoIntensity;
    float lfoRate;
    float noiseLevel;
    float resonance;
    float rhythmVolume;
    float tempo;
    float volume;
    float octave;
    
    int envelopeType;
    int lfoMode;
    int lfoTarget;
    int lfoWaveform;
    int ribbonMode;
    int waveform;
    
    // Update counter to refresh cache periodically
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
