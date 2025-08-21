#pragma once
#include "vcf/ms20.hpp"
#include "vcf/ladder.hpp"
#include "vcf/moog.hpp"
#include "vcf/filter_type.hpp"

namespace clonotribe {

class FilterProcessor final {
public:
    explicit FilterProcessor(MS20Filter& vcf) noexcept : vcf(&vcf), ladder(nullptr), moog(nullptr), filterType(FilterType::MS20) {}
    explicit FilterProcessor(LadderFilter& ladder) noexcept : vcf(nullptr), ladder(&ladder), moog(nullptr), filterType(FilterType::LADDER) {}
    explicit FilterProcessor(MoogFilter& moog) noexcept : vcf(nullptr), ladder(nullptr), moog(&moog), filterType(FilterType::MOOG) {}

    void setFilterType(FilterType type, MS20Filter* vcfPtr, LadderFilter* ladderPtr, MoogFilter* moogPtr) noexcept {
        filterType = type;
        vcf = vcfPtr;
        ladder = ladderPtr;
        moog = moogPtr;
    }

    [[nodiscard]] float process(float input, float cutoff, float resonance) noexcept {
        if (std::abs(input) < 1e-30f) input = ZERO;
        
        float smoothedCutoff = cutoff;
        float smoothedResonance = resonance;
        
        float cutoffDelta = std::abs(cutoff - lastCutoff);
        float resonanceDelta = std::abs(resonance - lastResonance);
        
        if (cutoffDelta > CUTOFF_THRESHOLD) {
            float smoothFactor = std::clamp(0.05f + cutoffDelta * 0.1f, 0.05f, 0.3f);
            smoothedCutoff = lastCutoff + (cutoff - lastCutoff) * smoothFactor;
            lastCutoff = smoothedCutoff;
        } else {
            lastCutoff = cutoff;
        }
        
        if (resonanceDelta > RESONANCE_THRESHOLD) {
            float smoothFactor = std::clamp(0.1f + resonanceDelta * 0.2f, 0.1f, 0.4f);
            smoothedResonance = lastResonance + (resonance - lastResonance) * smoothFactor;
            lastResonance = smoothedResonance;
        } else {
            lastResonance = resonance;
        }
        
        switch (filterType) {
            case FilterType::MS20:
                if (vcf) {
                    vcf->setCutoff(smoothedCutoff);
                    vcf->setResonance(smoothedResonance);
                    float output = vcf->process(input);
                    if (std::abs(output) < 1e-30f) output = ZERO;
                    return output;
                }
                break;
            case FilterType::LADDER:
                if (ladder) {
                    ladder->setCutoff(smoothedCutoff);
                    ladder->setResonance(smoothedResonance);
                    float output = ladder->process(input);
                    if (std::abs(output) < 1e-30f) output = ZERO;
                    return output;
                }
                break;
            case FilterType::MOOG:
                if (moog) {
                    moog->setCutoff(smoothedCutoff);
                    moog->setResonance(smoothedResonance);
                    float output = moog->process(input);
                    if (std::abs(output) < 1e-30f) output = ZERO;
                    return output;
                }
                break;
            default:
                break;
        }
        return ZERO;
    }

    void forceUpdate(float cutoff, float resonance) noexcept {
        switch (filterType) {
            case FilterType::MS20:
                if (vcf) {
                    vcf->setCutoff(cutoff);
                    vcf->setResonance(resonance);
                }
                break;
            case FilterType::LADDER:
                if (ladder) {
                    ladder->setCutoff(cutoff);
                    ladder->setResonance(resonance);
                }
                break;
            case FilterType::MOOG:
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
        lastCutoff = -ONE;
        lastResonance = -ONE;
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
    float lastCutoff = -ONE;
    float lastResonance = -ONE;
};
}
