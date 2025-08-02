#pragma once
#include <rack.hpp>

namespace clonotribe {

struct DrumParameterCache final {
    float kickTune = 0.0f;
    float kickDecay = 0.5f;
    float snareTune = 0.0f;
    float snareDecay = 0.5f;
    float hihatTune = 0.0f;
    float hihatDecay = 0.5f;
    
    static constexpr int UPDATE_INTERVAL = 32; // Update every 32 samples (~0.7ms at 44.1kHz)
    int updateCounter = 0;
    
    void update() noexcept {
        if (++updateCounter >= UPDATE_INTERVAL) {
            updateCounter = 0;
            
            // Update drum tuning parameters if they exist
            // These would correspond to drum-specific parameter IDs when available
            // For now, use default values optimized for each drum kit
            
            // TR-808 style tuning defaults
            kickTune = 0.0f;    // Centered tuning
            kickDecay = 0.6f;   // Medium decay for punch
            
            snareTune = 0.1f;   // Slightly higher for snap
            snareDecay = 0.4f;  // Shorter decay for crisp sound
            
            hihatTune = 0.8f;   // High tuning for brightness
            hihatDecay = 0.2f;  // Very short decay for tight sound
        }
    }
    
    void resetUpdateCounter() noexcept {
        updateCounter = 0;
    }
    
    // Get interpolated values for smooth parameter changes
    [[nodiscard]] float getKickTune() const noexcept { return kickTune; }
    [[nodiscard]] float getKickDecay() const noexcept { return kickDecay; }
    [[nodiscard]] float getSnareTune() const noexcept { return snareTune; }
    [[nodiscard]] float getSnareDecay() const noexcept { return snareDecay; }
    [[nodiscard]] float getHihatTune() const noexcept { return hihatTune; }
    [[nodiscard]] float getHihatDecay() const noexcept { return hihatDecay; }
};

}
