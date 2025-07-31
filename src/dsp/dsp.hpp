#pragma once
#include <rack.hpp>
#include "envelope.hpp"
#include "fastmath.hpp"
#include "lfo.hpp"
#include "noise.hpp"
#include "ribbon.hpp"
#include "sequencer.hpp"
#include "vcf.hpp"
#include "vco.hpp"

namespace dsp {
    using SchmittTrigger = rack::dsp::SchmittTrigger;
    using PulseGenerator = rack::dsp::PulseGenerator;
}
