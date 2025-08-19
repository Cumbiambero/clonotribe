#pragma once


#include "envelope.hpp"
#include "sequencer_state_manager.hpp"

namespace clonotribe {

struct ParameterCache {
    float cutoff;
    float accentGlideAmount;
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

    EnvelopeType envelopeType;
    SequencerStateManager::LFOMode lfoMode;
    SequencerStateManager::LFOTarget lfoTarget;
    SequencerStateManager::LFOWaveform lfoWaveform;

    int ribbonMode;
    int waveform;
    int updateCounter = 0;
    static constexpr int UPDATE_INTERVAL = 64;

    bool needsUpdate() {
        return (++updateCounter >= UPDATE_INTERVAL);
    }

    void resetUpdateCounter() {
        updateCounter = 0;
    }
};
}