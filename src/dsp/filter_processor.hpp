#pragma once
#include "vcf.hpp"

namespace clonotribe {

/**
 * High-performance filter processor with parameter caching
 * Reduces expensive filter parameter updates
 */
class FilterProcessor final {
private:
    VCF* filter;
    
    // Cached parameters
    float lastCutoff = -1.0f;
    float lastResonance = -1.0f;
    
    // Update thresholds for smooth operation
    static constexpr float CUTOFF_THRESHOLD = 0.001f;
    static constexpr float RESONANCE_THRESHOLD = 0.001f;
    
public:
    explicit FilterProcessor(VCF& vcf) noexcept : filter(&vcf) {}
    
    // High-performance filter processing with minimal parameter updates
    [[nodiscard]] float process(float input, float cutoff, float resonance) noexcept {
        // Only update parameters when they change significantly
        if (std::abs(cutoff - lastCutoff) > CUTOFF_THRESHOLD) {
            filter->setCutoff(cutoff);
            lastCutoff = cutoff;
        }
        
        if (std::abs(resonance - lastResonance) > RESONANCE_THRESHOLD) {
            filter->setResonance(resonance);
            lastResonance = resonance;
        }
        
        return filter->process(input);
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

} // namespace clonotribe
