#pragma once
#include "vcf/ms20.hpp"
#include "vcf/ladder.hpp"
#include "vcf/moog.hpp"
#include "vcf/filter_type.hpp"

namespace clonotribe {

class FilterProcessor final {
public:
    explicit FilterProcessor(MS20Filter& vcf) noexcept : vcf(&vcf), ladder(nullptr), moog(nullptr), filterType(FILTER_MS20) {}
    explicit FilterProcessor(LadderFilter& ladder) noexcept : vcf(nullptr), ladder(&ladder), moog(nullptr), filterType(FILTER_LADDER) {}
    explicit FilterProcessor(MoogFilter& moog) noexcept : vcf(nullptr), ladder(nullptr), moog(&moog), filterType(FILTER_MOOG) {}

    void setFilterType(FilterType type, MS20Filter* vcfPtr, LadderFilter* ladderPtr, MoogFilter* moogPtr) noexcept {
        filterType = type;
        vcf = vcfPtr;
        ladder = ladderPtr;
        moog = moogPtr;
    }

    [[nodiscard]] float process(float input, float cutoff, float resonance) noexcept {
        if (std::abs(input) < 1e-30f) input = 0.0f;
        
        float smoothedCutoff = cutoff;
        float smoothedResonance = resonance;
        
        if (std::abs(cutoff - lastCutoff) > CUTOFF_THRESHOLD) {
            smoothedCutoff = lastCutoff + (cutoff - lastCutoff) * 0.2f;
            lastCutoff = smoothedCutoff;
        } else {
            lastCutoff = cutoff;
        }
        
        if (std::abs(resonance - lastResonance) > RESONANCE_THRESHOLD) {
            smoothedResonance = lastResonance + (resonance - lastResonance) * 0.2f;
            lastResonance = smoothedResonance;
        } else {
            lastResonance = resonance;
        }
        
        switch (filterType) {
            case FILTER_MS20:
                if (vcf) {
                    vcf->setCutoff(smoothedCutoff);
                    vcf->setResonance(smoothedResonance);
                    float output = vcf->process(input);
                    if (std::abs(output) < 1e-30f) output = 0.0f;
                    return output;
                }
                break;
            case FILTER_LADDER:
                if (ladder) {
                    ladder->setCutoff(smoothedCutoff);
                    ladder->setResonance(smoothedResonance);
                    float output = ladder->process(input);
                    if (std::abs(output) < 1e-30f) output = 0.0f;
                    return output;
                }
                break;
            case FILTER_MOOG:
                if (moog) {
                    moog->setCutoff(smoothedCutoff);
                    moog->setResonance(smoothedResonance);
                    float output = moog->process(input);
                    if (std::abs(output) < 1e-30f) output = 0.0f;
                    return output;
                }
                break;
            default:
                break;
        }
        return 0.0f;
    }

    void forceUpdate(float cutoff, float resonance) noexcept {
        switch (filterType) {
            case FILTER_MS20:
                if (vcf) {
                    vcf->setCutoff(cutoff);
                    vcf->setResonance(resonance);
                }
                break;
            case FILTER_LADDER:
                if (ladder) {
                    ladder->setCutoff(cutoff);
                    ladder->setResonance(resonance);
                }
                break;
            case FILTER_MOOG:
                if (moog) {
                    moog->setCutoff(cutoff);
                    moog->setResonance(resonance);
                }
                break;
            default:
                break;
        }
        lastCutoff = cutoff;
        lastResonance = resonance;
    }

    void reset() noexcept {
        lastCutoff = -1.0f;
        lastResonance = -1.0f;
    }

    void setPointers(MS20Filter* vcfPtr, LadderFilter* ladderPtr, MoogFilter* moogPtr) noexcept {
        vcf = vcfPtr;
        ladder = ladderPtr;
        moog = moogPtr;
    }

    void setType(FilterType type) noexcept { filterType = type; }

    FilterType getType() const noexcept { return filterType; }

private:
    static constexpr float CUTOFF_THRESHOLD = 0.01f;
    static constexpr float RESONANCE_THRESHOLD = 0.01f;
    MS20Filter* vcf;
    LadderFilter* ladder;
    MoogFilter* moog;
    FilterType filterType;
    float lastCutoff = -1.0f;
    float lastResonance = -1.0f;
};
}
