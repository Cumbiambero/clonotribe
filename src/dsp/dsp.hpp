#pragma once
#include <rack.hpp>
#include "envelope.hpp"
#include "fastmath.hpp"
#include "lfo.hpp"
#include "noise.hpp"
#include "ribbon.hpp"
#include "sequencer/sequencer.hpp"
#include "vco.hpp"
#include "distortion.hpp"

namespace dsp {
    using SchmittTrigger = rack::dsp::SchmittTrigger;
    using PulseGenerator = rack::dsp::PulseGenerator;
}