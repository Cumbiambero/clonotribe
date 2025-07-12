#pragma once
#include <rack.hpp>

using namespace rack;

namespace clonotribe {

struct ADSR {
  enum Stage { ATTACK, DECAY, SUSTAIN, RELEASE, OFF };
  Stage stage = OFF;
  float value = 0.f;
  float attack = 0.1f;
  float decay = 0.1f;
  float sustain = 0.7f;
  float releaseTime = 0.5f;

  void setAttack(float a) { attack = clamp(a, 0.001f, 10.f); }
  void setDecay(float d) { decay = clamp(d, 0.001f, 10.f); }
  void setSustain(float s) { sustain = clamp(s, 0.f, 1.f); }
  void setRelease(float r) { releaseTime = clamp(r, 0.001f, 10.f); }

  void trigger() { stage = ATTACK; }

  void gateOff() {
    if (stage != OFF) stage = RELEASE;
  }

  float process(float sampleTime) {
    switch (stage) {
      case ATTACK:
        value += sampleTime / attack;
        if (value >= 1.f) {
          value = 1.f;
          stage = DECAY;
        }
        break;
      case DECAY:
        value -= sampleTime / decay;
        if (value <= sustain) {
          value = sustain;
          stage = SUSTAIN;
        }
        break;
      case SUSTAIN:
        value = sustain;
        break;
      case RELEASE:
        value -= sampleTime / releaseTime;
        if (value <= 0.f) {
          value = 0.f;
          stage = OFF;
        }
        break;
      case OFF:
        value = 0.f;
        break;
    }
    return value;
  }
};
}