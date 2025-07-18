#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../../src/dsp/sequencer_state.hpp"

TEST_CASE("SequencerState step advance and drum trigger") {
    SequencerState sm;
    sm.setStepActive(0, true);
    sm.setDrumForStep(0, SequencerState::Drum::BassDrum);
    CHECK(sm.getCurrentStep() == 0);
    sm.advance();
    CHECK(sm.getCurrentStep() == 1);
    sm.setStepActive(1, true);
    sm.setDrumForStep(1, SequencerState::Drum::Snare);
    sm.advance();
    CHECK(sm.getCurrentStep() == 2);
    sm.triggerDrum(SequencerState::Drum::HiHat);
    CHECK(sm.getStepState(2).drum == SequencerState::Drum::HiHat);
}
