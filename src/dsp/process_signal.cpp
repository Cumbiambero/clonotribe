#include "../clonotribe.hpp"
#include "envelope.hpp"

float Clonotribe::processEnvelope(EnvelopeType envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate) {
    float envValue = 1.0f;
    bool useGate = (finalSequencerGate > 1.0f);
    switch (envelopeType) {
        case EnvelopeType::ATTACK:
            envelope.setAttack(0.1f);
            envelope.setDecay(0.1f);
            envelope.setSustain(1.0f);
            envelope.setRelease(0.1f);
            envValue = envelope.process(sampleTime);
            break;
        case EnvelopeType::GATE:
            envValue = useGate ? 1.0f : 0.0f;
            break;
        case EnvelopeType::DECAY:
            envelope.setAttack(0.001f);
            envelope.setDecay(0.5f);
            envelope.setSustain(0.0f);
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
    float volumeModulation = 1.0f + (ribbonVolumeAutomation * 0.5f);
    volumeModulation = std::clamp(volumeModulation, 0.1f, 2.0f);
    
    float synthOutput = filteredSignal * volume * envValue * volumeModulation;
    
    if (distortion > 0.0f) {
        float driveGain = 1.0f + (distortion * 2.0f);
        float drivenSignal = synthOutput * driveGain;
        float distortedSignal = distortionProcessor.process(drivenSignal, distortion);
        float outputLevel = std::abs(synthOutput);
        float distortedLevel = std::abs(distortedSignal);
        
        if (outputLevel > 0.0001f && distortedLevel > outputLevel * 3.0f) {
            float excessGain = distortedLevel / (outputLevel * 2.5f);
            float compressionFactor = 1.0f + std::sqrt(excessGain - 1.0f) * 0.5f;
            distortedSignal /= compressionFactor;
        }
        
        synthOutput = distortedSignal;
    }
    
    if (delayAmount > 0.0f && delayTime > 0.001f) {
        synthOutput = delayProcessor.process(synthOutput, delayClock, delayTime, delayAmount);
    }

    float drumMix = 0.0f;
    if (rhythmVolume > 0.0f) {
        float kickOut = drumProcessor.processKick(0.0f, 0.0f, noiseGenerator);
        float snareOut = drumProcessor.processSnare(0.0f, 0.0f, noiseGenerator);
        float hihatOut = drumProcessor.processHihat(0.0f, 0.0f, noiseGenerator);
        
        drumMix = (kickOut * 0.7f + snareOut * 0.6f + hihatOut * 0.5f) * rhythmVolume;
        
        outputs[OUTPUT_BASSDRUM_CONNECTOR].setVoltage(std::clamp(kickOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
        outputs[OUTPUT_SNARE_CONNECTOR].setVoltage(std::clamp(snareOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
        outputs[OUTPUT_HIHAT_CONNECTOR].setVoltage(std::clamp(hihatOut * rhythmVolume * 4.0f, -10.0f, 10.0f));
    } else {
        outputs[OUTPUT_BASSDRUM_CONNECTOR].setVoltage(0.0f);
        outputs[OUTPUT_SNARE_CONNECTOR].setVoltage(0.0f);
        outputs[OUTPUT_HIHAT_CONNECTOR].setVoltage(0.0f);
    }
    
    outputs[OUTPUT_SYNTH_CONNECTOR].setVoltage(std::clamp(synthOutput * 4.0f, -10.0f, 10.0f));
    
    float synthLevel = synthOutput * 0.8f;
    return synthLevel + drumMix;
}
