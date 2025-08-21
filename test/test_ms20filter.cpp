
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../src/dsp/vcf/ms20.hpp"
#include "../src/dsp/fastmath.hpp"
#include <cmath>

using clonotribe::MS20Filter;

TEST_CASE("MS20Filter silence") {
    MS20Filter filter;
    filter.setSampleRate(44100.0f);
    filter.setCutoff(0.5f);
    filter.setResonance(0.5f);
    filter.setActive(true);
    for (int i = 0; i < 32; ++i) {
        CHECK(filter.process(ZERO) == doctest::Approx(ZERO).epsilon(1e-6f));
    }
}

TEST_CASE("MS20Filter impulse response") {
    MS20Filter filter;
    filter.setSampleRate(44100.0f);
    filter.setCutoff(0.7f);
    filter.setResonance(ZERO);
    filter.setActive(true);
    auto out = filter.process(ONE);
    CHECK(std::isfinite(out));
    CHECK(std::abs(out) > ZERO);
}

TEST_CASE("MS20Filter resonance self-oscillation") {
    MS20Filter filter;
    filter.setSampleRate(44100.0f);
    filter.setCutoff(0.5f);
    filter.setResonance(ONE);
    filter.setActive(true);
    float maxVal = ZERO;
    for (int i = 0; i < 2000; ++i) {
        maxVal = std::max(maxVal, std::abs(filter.process(ZERO)));
    }
    CHECK(maxVal > 0.01f);
}

TEST_CASE("MS20Filter cutoff sweep") {
    MS20Filter filter;
    filter.setSampleRate(44100.0f);
    filter.setResonance(ZERO);
    filter.setActive(true);
    for (float cutoff = ZERO; cutoff <= ONE; cutoff += ONE) {
        filter.setCutoff(cutoff);
        auto out = filter.process(ONE);
        CHECK(std::isfinite(out));
    }
}

TEST_CASE("MS20Filter denormal handling and stability") {
    MS20Filter f; f.setSampleRate(44100.f); f.setActive(true); f.setCutoff(0.3f); f.setResonance(0.0f);
    for (int i = 0; i < 2048; ++i) {
        auto y = f.process(1e-20f);
        CHECK(std::isfinite(y));
        CHECK(std::abs(y) < 1e-6f);
    }
}
