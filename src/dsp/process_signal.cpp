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
    float rhythmVolume, float sampleTime,
    KickDrum& kickDrum, SnareDrum& snareDrum, HiHat& hiHat, NoiseGenerator& noiseGenerator
) {
    // Apply VCA (volume and envelope) with ribbon controller volume automation
    float volumeModulation = 1.0f + (ribbonVolumeAutomation * 0.5f); // ±50% volume change
    volumeModulation = clamp(volumeModulation, 0.1f, 2.0f);
    float finalOutput = filteredSignal * volume * envValue * volumeModulation;

    // Process drums and mix with synth output
    float drumMix = 0.0f;
    if (rhythmVolume > 0.0f) {
        float kickOut = kickDrum.process(sampleTime);
        float snareOut = snareDrum.process(sampleTime, noiseGenerator);
        float hihatOut = hiHat.process(sampleTime, noiseGenerator);
        drumMix = (kickOut + snareOut + hihatOut) * rhythmVolume;
    }
    finalOutput += drumMix;  // mix synth and drums
    return finalOutput;
}
