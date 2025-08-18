#pragma once
#include "dsp.hpp"
#include "parameter_cache.hpp"

namespace clonotribe {

class AudioProcessor {
public:
    AudioProcessor() = default;
    
    struct ProcessContext {
        float sampleTime;
        float sampleRate;
        float inputPitch;
        float inputGate;
        float ribbonCV;
        float ribbonGate;
        bool ribbonTouching;
        bool gateTimeHeld;
        ParameterCache& params;
    };
    
    struct AudioOutput {
        float mainOutput;
        float cvOutput;
        float gateOutput;
        float syncOutput;
    };
    
    AudioOutput process(const ProcessContext& ctx, 
                       VCO& vco, VCF& vcf, LFO& lfo, Envelope& envelope,
                       NoiseGenerator& noiseGen, Sequencer& sequencer,
                       drumkits::KickDrum& kick, drumkits::SnareDrum& snare, 
                       drumkits::HiHat& hihat);

private:
    float processVCO(const ProcessContext& ctx, VCO& vco, float pitch, float pitchMod);
    float processFilter(const ProcessContext& ctx, VCF& vcf, float input, float cutoffMod);
    float processEnvelope(const ProcessContext& ctx, Envelope& envelope, float gate);
    float processLFO(const ProcessContext& ctx, LFO& lfo);
    float processDrums(const ProcessContext& ctx, 
                      drumkits::KickDrum& kick, drumkits::SnareDrum& snare, 
                      drumkits::HiHat& hihat, NoiseGenerator& noiseGen);
};

}