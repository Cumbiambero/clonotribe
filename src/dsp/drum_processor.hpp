#pragma once
#include "drumkits/base/base.hpp"
#include "drumkits/original/kickdrum.hpp"
#include "drumkits/original/snaredrum.hpp"
#include "drumkits/original/hihat.hpp"
#include "drumkits/tr808/kickdrum.hpp"
#include "drumkits/tr808/snaredrum.hpp"
#include "drumkits/tr808/hihat.hpp"
#include "drumkits/latin/kickdrum.hpp"
#include "drumkits/latin/snaredrum.hpp"
#include "drumkits/latin/hihat.hpp"
#include "noise.hpp"

namespace clonotribe {

class DrumProcessor {
public:
    enum class DrumKitType {
        ORIGINAL = 0,
        TR808 = 1,
        LATIN = 2
    };
    
    DrumProcessor() {
        setDrumKit(DrumKitType::ORIGINAL);
        setSampleRate(44100.0f);
    }
    
    void setSampleRate(float sampleRate) {
        currentSampleRate = sampleRate;        
        kickOriginal.setSampleRate(sampleRate);
        snareOriginal.setSampleRate(sampleRate);
        hihatOriginal.setSampleRate(sampleRate);
        kickTR808.setSampleRate(sampleRate);
        snareTR808.setSampleRate(sampleRate);
        hihatTR808.setSampleRate(sampleRate);
        kickLatin.setSampleRate(sampleRate);
        snareLatin.setSampleRate(sampleRate);
        hihatLatin.setSampleRate(sampleRate);
    }
    
    void setDrumKit(DrumKitType kit) {
        currentKit = kit;
        resetAllDrums();
    }
    
    void resetAllDrums() {
        kickOriginal.reset();
        snareOriginal.reset();
        hihatOriginal.reset();
        kickTR808.reset();
        snareTR808.reset();
        hihatTR808.reset();
        kickLatin.reset();
        snareLatin.reset();
        hihatLatin.reset();
    }
    
    void triggerKick() {
        switch (currentKit) {
            case DrumKitType::TR808: kickTR808.reset(); break;
            case DrumKitType::LATIN: kickLatin.reset(); break;
            default: kickOriginal.reset(); break;
        }
    }
    
    void triggerSnare() {
        switch (currentKit) {
            case DrumKitType::TR808: snareTR808.reset(); break;
            case DrumKitType::LATIN: snareLatin.reset(); break;
            default: snareOriginal.reset(); break;
        }
    }
    
    void triggerHihat() {
        switch (currentKit) {
            case DrumKitType::TR808: hihatTR808.reset(); break;
            case DrumKitType::LATIN: hihatLatin.reset(); break;
            default: hihatOriginal.reset(); break;
        }
    }
    
    float processKick(float trig, float accent, NoiseGenerator& noise) {
        switch (currentKit) {
            case DrumKitType::TR808: return kickTR808.process(trig, accent, noise);
            case DrumKitType::LATIN: return kickLatin.process(trig, accent, noise);
            default: return kickOriginal.process(trig, accent, noise);
        }
    }
    
    float processSnare(float trig, float accent, NoiseGenerator& noise) {
        switch (currentKit) {
            case DrumKitType::TR808: return snareTR808.process(trig, accent, noise);
            case DrumKitType::LATIN: return snareLatin.process(trig, accent, noise);
            default: return snareOriginal.process(trig, accent, noise);
        }
    }
    
    float processHihat(float trig, float accent, NoiseGenerator& noise) {
        switch (currentKit) {
            case DrumKitType::TR808: return hihatTR808.process(trig, accent, noise);
            case DrumKitType::LATIN: return hihatLatin.process(trig, accent, noise);
            default: return hihatOriginal.process(trig, accent, noise);
        }
    }

private:
    DrumKitType currentKit = DrumKitType::ORIGINAL;
    float currentSampleRate = 44100.0f;
    
    drumkits::original::KickDrum kickOriginal;
    drumkits::original::SnareDrum snareOriginal;
    drumkits::original::HiHat hihatOriginal;
    drumkits::tr808::KickDrum kickTR808;
    drumkits::tr808::SnareDrum snareTR808;
    drumkits::tr808::HiHat hihatTR808;
    drumkits::latin::KickDrum kickLatin;
    drumkits::latin::SnareDrum snareLatin;
    drumkits::latin::HiHat hihatLatin;
};
}