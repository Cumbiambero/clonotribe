#include "../clonotribe.hpp"
#include "envelope.hpp"

float Clonotribe::processEnvelope(Envelope::Type envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate) {
    float envValue = ONE;
    bool useGate = (finalSequencerGate > ONE);
    switch (envelopeType) {
        case Envelope::Type::ATTACK:
            envelope.setAttack(0.1f);
            envelope.setDecay(0.1f);
            envelope.setSustain(ONE);
            envelope.setRelease(0.1f);
            envValue = envelope.process(sampleTime);
            break;
        case Envelope::Type::GATE:
            envValue = useGate ? ONE : ZERO;
            break;
        case Envelope::Type::DECAY:
            envelope.setAttack(0.001f);
            envelope.setDecay(0.5f);
            envelope.setSustain(ZERO);
            envelope.setRelease(0.001f);
            envValue = envelope.process(sampleTime);
            break;
        default:
            break;
    }
    return envValue;
}

[[nodiscard]] float Clonotribe::processOutput(
    float filteredSignal, float volume, float envValue, float ribbonVolumeAutomation,
    float rhythmVolume, float sampleTime, NoiseGenerator& noiseGenerator, int currentStep, float distortion,
    float delayClock, float delayTime, float delayAmount
) {
    float volumeModulation = ONE + (ribbonVolumeAutomation * HALF);
    volumeModulation = std::clamp(volumeModulation, 0.1f, TWO);
    
    float synthOutput = filteredSignal * volume * envValue * volumeModulation;
    
    if (distortion > ZERO) {
        float driveGain = ONE + (distortion * TWO);
        float drivenSignal = synthOutput * driveGain;
        float distortedSignal = distortionProcessor.process(drivenSignal, distortion);
        float outputLevel = std::abs(synthOutput);
        float distortedLevel = std::abs(distortedSignal);
        
        if (outputLevel > 0.0001f && distortedLevel > outputLevel * 3.0f) {
            float excessGain = distortedLevel / (outputLevel * 2.5f);
            float compressionFactor = ONE + std::sqrt(excessGain - ONE) * HALF;
            distortedSignal /= compressionFactor;
        }
        
        synthOutput = dcBlockerPostDist.processAggressive(distortedSignal);
    }
    
    if (delayAmount > ZERO && delayTime > 0.001f) {
        synthOutput = delayProcessor.process(synthOutput, delayClock, delayTime, delayAmount);
    }

    float drumMix = ZERO;
    if (rhythmVolume > ZERO) {
        float kickOut = drumProcessor.processKick(0.0f, ZERO, noiseGenerator);
        float snareOut = drumProcessor.processSnare(0.0f, ZERO, noiseGenerator);
        float hihatOut = drumProcessor.processHihat(0.0f, ZERO, noiseGenerator);
        
        drumMix = (kickOut * 0.7f + snareOut * 0.6f + hihatOut * HALF) * rhythmVolume;
        
        outputs[OUTPUT_BASSDRUM_CONNECTOR].setVoltage(std::clamp(kickOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
        outputs[OUTPUT_SNARE_CONNECTOR].setVoltage(std::clamp(snareOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
        outputs[OUTPUT_HIHAT_CONNECTOR].setVoltage(std::clamp(hihatOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
    } else {
        outputs[OUTPUT_BASSDRUM_CONNECTOR].setVoltage(ZERO);
        outputs[OUTPUT_SNARE_CONNECTOR].setVoltage(ZERO);
        outputs[OUTPUT_HIHAT_CONNECTOR].setVoltage(ZERO);
    }
    
    outputs[OUTPUT_SYNTH_CONNECTOR].setVoltage(std::clamp(synthOutput * 4.0f, -10.0f, 10.0f));
    
    float synthLevel = synthOutput * 0.8f;
    return synthLevel + drumMix;
}
