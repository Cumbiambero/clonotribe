#pragma once
#include "../../noise.hpp"

namespace drumkits {

struct KickDrum {
    virtual float process(float trig, float accent, clonotribe::NoiseGenerator& noise) = 0;
    virtual void reset() = 0;
    virtual void setSampleRate(float sampleRate) = 0;
    virtual ~KickDrum() = default;
};

struct SnareDrum {
    virtual float process(float trig, float accent, clonotribe::NoiseGenerator& noise) = 0;
    virtual void reset() = 0;
    virtual void setSampleRate(float sampleRate) = 0;
    virtual ~SnareDrum() = default;
};

struct HiHat {
    virtual float process(float trig, float accent, clonotribe::NoiseGenerator& noise) = 0;
    virtual void reset() = 0;
    virtual void setSampleRate(float sampleRate) = 0;
    virtual ~HiHat() = default;
};

} 
