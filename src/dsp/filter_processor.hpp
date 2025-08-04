#pragma once
#include "vcf.hpp"

namespace clonotribe {

class FilterProcessor final {
private:
    static constexpr float CUTOFF_THRESHOLD = 0.01f;
    static constexpr float RESONANCE_THRESHOLD = 0.01f;

    VCF* filter;
    
    float lastCutoff = -1.0f;
    float lastResonance = -1.0f;
    
public:
    explicit FilterProcessor(VCF& vcf) noexcept : filter(&vcf) {}
    
    // High-performance filter processing with minimal parameter updates
    [[nodiscard]] float process(float input, float cutoff, float resonance) noexcept {
        // Check for denormal inputs
        if (std::abs(input) < 1e-30f) input = 0.0f;
        
        // Only update parameters when they change significantly
        if (std::abs(cutoff - lastCutoff) > CUTOFF_THRESHOLD) {
            filter->setCutoff(cutoff);
            lastCutoff = cutoff;
        }
        
        if (std::abs(resonance - lastResonance) > RESONANCE_THRESHOLD) {
            filter->setResonance(resonance);
            lastResonance = resonance;
        }
        
        float output = filter->process(input);
        
        // Prevent denormal output
        if (std::abs(output) < 1e-30f) output = 0.0f;
        
        return output;
    }
    
    // Force parameter update (useful for initialization)
    void forceUpdate(float cutoff, float resonance) noexcept {
        filter->setCutoff(cutoff);
        filter->setResonance(resonance);
        lastCutoff = cutoff;
        lastResonance = resonance;
    }
    
    // Reset cached values
    void reset() noexcept {
        lastCutoff = -1.0f;
        lastResonance = -1.0f;
    }
};
}
