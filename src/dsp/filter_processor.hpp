#pragma once
#include "vcf/ms20.hpp"
#include "vcf/ladder.hpp"
#include "vcf/filter_type.hpp"

namespace clonotribe {

class FilterProcessor final {
public:
    explicit FilterProcessor(MS20Filter& vcf) noexcept : vcf(&vcf), ladder(nullptr), filterType(FILTER_MS20) {}
    explicit FilterProcessor(LadderFilter& ladder) noexcept : vcf(nullptr), ladder(&ladder), filterType(FILTER_LADDER) {}

    void setFilterType(FilterType type, MS20Filter* vcfPtr, LadderFilter* ladderPtr) noexcept {
        filterType = type;
        vcf = vcfPtr;
        ladder = ladderPtr;
    }

    [[nodiscard]] float process(float input, float cutoff, float resonance) noexcept {
        if (std::abs(input) < 1e-30f) input = 0.0f;
        if (filterType == FILTER_MS20 && vcf) {
            if (std::abs(cutoff - lastCutoff) > CUTOFF_THRESHOLD) {
                vcf->setCutoff(cutoff);
                lastCutoff = cutoff;
            }
            if (std::abs(resonance - lastResonance) > RESONANCE_THRESHOLD) {
                vcf->setResonance(resonance);
                lastResonance = resonance;
            }
            float output = vcf->process(input);
            if (std::abs(output) < 1e-30f) output = 0.0f;
            return output;
        } else if (filterType == FILTER_LADDER && ladder) {
            if (std::abs(cutoff - lastCutoff) > CUTOFF_THRESHOLD) {
                ladder->setCutoff(cutoff);
                lastCutoff = cutoff;
            }
            if (std::abs(resonance - lastResonance) > RESONANCE_THRESHOLD) {
                ladder->setResonance(resonance);
                lastResonance = resonance;
            }
            float output = ladder->process(input);
            if (std::abs(output) < 1e-30f) output = 0.0f;
            return output;
        }
        return 0.0f;
    }

    void forceUpdate(float cutoff, float resonance) noexcept {
        if (filterType == FILTER_MS20 && vcf) {
            vcf->setCutoff(cutoff);
            vcf->setResonance(resonance);
        } else if (filterType == FILTER_LADDER && ladder) {
            ladder->setCutoff(cutoff);
            ladder->setResonance(resonance);
        }
        lastCutoff = cutoff;
        lastResonance = resonance;
    }

    void reset() noexcept {
        lastCutoff = -1.0f;
        lastResonance = -1.0f;
    }

    void setPointers(MS20Filter* vcfPtr, LadderFilter* ladderPtr) noexcept {
        vcf = vcfPtr;
        ladder = ladderPtr;
    }

    void setType(FilterType type) noexcept { filterType = type; }

    FilterType getType() const noexcept { return filterType; }

private:
    static constexpr float CUTOFF_THRESHOLD = 0.01f;
    static constexpr float RESONANCE_THRESHOLD = 0.01f;
    MS20Filter* vcf;
    LadderFilter* ladder;
    FilterType filterType;
    float lastCutoff = -1.0f;
    float lastResonance = -1.0f;
};
}
