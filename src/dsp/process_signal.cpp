#include "../clonotribe.hpp"

float Clonotribe::processEnvelope(int envelopeType, Envelope& envelope, float sampleTime, float finalSequencerGate) {
    float envValue = 1.0f;
    bool useGate = (finalSequencerGate > 1.0f);
    switch (envelopeType) {
        case 0: // Attack
            envelope.setAttack(0.1f);
            envelope.setDecay(0.1f);
            envelope.setSustain(1.0f);
            envelope.setRelease(0.1f);
            envValue = envelope.process(sampleTime);
            break;
        case 1: // Gate
            envValue = useGate ? 1.0f : 0.0f;
            break;
        case 2: // Decay
            envelope.setAttack(0.001f);
            envelope.setDecay(0.5f);
            envelope.setSustain(0.0f);
            envelope.setRelease(0.001f);
            envValue = envelope.process(sampleTime);
            break;
    }
    return envValue;
}

float Clonotribe::processOutput(
    float filteredSignal, float volume, float envValue, float ribbonVolumeAutomation,
    float rhythmVolume, float sampleTime, NoiseGenerator& noiseGenerator, int currentStep, float distortion
) {
    float volumeModulation = 1.0f + (ribbonVolumeAutomation * 0.5f);
    volumeModulation = std::clamp(volumeModulation, 0.1f, 2.0f);
    float synthOutput = filteredSignal * volume * envValue * volumeModulation;

    if (distortion > 0.0f) {
        synthOutput = distortionProcessor.process(synthOutput, distortion);
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
