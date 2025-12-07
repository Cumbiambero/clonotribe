/**
 * MS-20 Filter Analysis and Improvement Tool
 * 
 * This tool analyzes the current MS20Filter implementation and compares it
 * against expected characteristics of the original Korg MS-20 filter:
 * 
 * Key MS-20 Filter Characteristics:
 * 1. Sallen-Key topology with OTAs (LM13700-based in original)
 * 2. 12dB/octave lowpass filter
 * 3. Self-oscillation at high resonance (creates sine-like tone)
 * 4. Asymmetric saturation characteristic (more aggressive than Moog)
 * 5. The Monotribe uses a simplified version with "whistling" character
 * 6. Non-linear cutoff response - more sensitive at low values
 * 7. Resonance "peak" knob that goes into self-oscillation
 */

#include <algorithm>
#include "../src/dsp/vcf/ms20.hpp"
#include "../src/dsp/fastmath.hpp"
#include "../src/constants.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>
#include <string>
#include <numeric>

using namespace clonotribe;

// Configuration
constexpr float SAMPLE_RATE = 44100.0f;
constexpr int ANALYSIS_SAMPLES = 8192;

// Analysis structure
struct FilterAnalysis {
    float cutoffParam;
    float resonanceParam;
    float dcOffset;
    float rmsLevel;
    float peakLevel;
    float dominantFreq;
    bool selfOscillating;
    float harmonicContent;
};

// FFT-free frequency estimation using zero-crossing
float estimateFrequency(const std::vector<float>& samples, float sampleRate) {
    int crossings = 0;
    for (size_t i = 1; i < samples.size(); ++i) {
        if ((samples[i-1] < 0 && samples[i] >= 0) || 
            (samples[i-1] >= 0 && samples[i] < 0)) {
            crossings++;
        }
    }
    return (crossings / 2.0f) * sampleRate / samples.size();
}

// Calculate RMS level
float calculateRMS(const std::vector<float>& samples) {
    float sum = 0;
    for (float s : samples) {
        sum += s * s;
    }
    return std::sqrt(sum / samples.size());
}

// Calculate DC offset
float calculateDC(const std::vector<float>& samples) {
    float sum = 0;
    for (float s : samples) {
        sum += s;
    }
    return sum / samples.size();
}

// Calculate peak level
float calculatePeak(const std::vector<float>& samples) {
    float peak = 0;
    for (float s : samples) {
        peak = std::max(peak, std::abs(s));
    }
    return peak;
}

// Test self-oscillation behavior
FilterAnalysis analyzeSelfOscillation(float cutoffParam, float resonanceParam) {
    MS20Filter filter;
    filter.setSampleRate(SAMPLE_RATE);
    filter.setCutoff(cutoffParam);
    filter.setResonance(resonanceParam);
    filter.setActive(true);
    
    // Warm up
    for (int i = 0; i < 1000; ++i) {
        filter.process(ZERO);
    }
    
    // Collect samples
    std::vector<float> samples(ANALYSIS_SAMPLES);
    for (int i = 0; i < ANALYSIS_SAMPLES; ++i) {
        samples[i] = filter.process(ZERO);
    }
    
    FilterAnalysis result;
    result.cutoffParam = cutoffParam;
    result.resonanceParam = resonanceParam;
    result.dcOffset = calculateDC(samples);
    result.rmsLevel = calculateRMS(samples);
    result.peakLevel = calculatePeak(samples);
    result.dominantFreq = estimateFrequency(samples, SAMPLE_RATE);
    result.selfOscillating = result.rmsLevel > 0.01f;
    
    // Estimate harmonic content (ratio of peak to RMS, pure sine = sqrt(2))
    if (result.rmsLevel > 0.001f) {
        result.harmonicContent = result.peakLevel / result.rmsLevel / std::sqrt(2.0f);
    } else {
        result.harmonicContent = 0;
    }
    
    return result;
}

// Test filter response to impulse
FilterAnalysis analyzeImpulseResponse(float cutoffParam, float resonanceParam) {
    MS20Filter filter;
    filter.setSampleRate(SAMPLE_RATE);
    filter.setCutoff(cutoffParam);
    filter.setResonance(resonanceParam);
    filter.setActive(true);
    
    // Send impulse
    std::vector<float> samples(ANALYSIS_SAMPLES);
    samples[0] = filter.process(ONE);
    for (int i = 1; i < ANALYSIS_SAMPLES; ++i) {
        samples[i] = filter.process(ZERO);
    }
    
    FilterAnalysis result;
    result.cutoffParam = cutoffParam;
    result.resonanceParam = resonanceParam;
    result.dcOffset = calculateDC(samples);
    result.rmsLevel = calculateRMS(samples);
    result.peakLevel = calculatePeak(samples);
    result.dominantFreq = estimateFrequency(samples, SAMPLE_RATE);
    result.selfOscillating = false;
    result.harmonicContent = (result.rmsLevel > 0.001f) ? 
        result.peakLevel / result.rmsLevel / std::sqrt(2.0f) : 0;
    
    return result;
}

// Test continuous signal filtering
FilterAnalysis analyzeContinuousSignal(float cutoffParam, float resonanceParam) {
    MS20Filter filter;
    filter.setSampleRate(SAMPLE_RATE);
    filter.setCutoff(cutoffParam);
    filter.setResonance(resonanceParam);
    filter.setActive(true);
    
    // Input: sawtooth at 100Hz (typical bass note)
    float phase = 0;
    float phaseInc = 100.0f / SAMPLE_RATE;
    
    // Warm up
    for (int i = 0; i < 4410; ++i) {
        float input = phase * 2.0f - 1.0f;
        filter.process(input);
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
    
    // Collect samples
    std::vector<float> samples(ANALYSIS_SAMPLES);
    for (int i = 0; i < ANALYSIS_SAMPLES; ++i) {
        float input = phase * 2.0f - 1.0f;
        samples[i] = filter.process(input);
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
    
    FilterAnalysis result;
    result.cutoffParam = cutoffParam;
    result.resonanceParam = resonanceParam;
    result.dcOffset = calculateDC(samples);
    result.rmsLevel = calculateRMS(samples);
    result.peakLevel = calculatePeak(samples);
    result.dominantFreq = estimateFrequency(samples, SAMPLE_RATE);
    result.selfOscillating = false;
    result.harmonicContent = (result.rmsLevel > 0.001f) ? 
        result.peakLevel / result.rmsLevel / std::sqrt(2.0f) : 0;
    
    return result;
}

void printAnalysis(const std::string& testName, const FilterAnalysis& a) {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << testName << ": cutoff=" << a.cutoffParam 
              << " reso=" << a.resonanceParam
              << " | DC=" << a.dcOffset
              << " RMS=" << a.rmsLevel
              << " Peak=" << a.peakLevel
              << " Freq=" << a.dominantFreq << "Hz"
              << " SelfOsc=" << (a.selfOscillating ? "YES" : "NO")
              << " Harmonic=" << a.harmonicContent
              << "\n";
}

void writeWaveform(const std::string& filename, float cutoff, float resonance, 
                   bool selfOsc = false) {
    MS20Filter filter;
    filter.setSampleRate(SAMPLE_RATE);
    filter.setCutoff(cutoff);
    filter.setResonance(resonance);
    filter.setActive(true);
    
    std::ofstream file(filename);
    file << "# MS20 Filter Waveform: cutoff=" << cutoff << " resonance=" << resonance << "\n";
    file << "# Sample, Input, Output\n";
    
    float phase = 0;
    float phaseInc = 200.0f / SAMPLE_RATE;  // 200Hz input for sawtooth
    
    for (int i = 0; i < ANALYSIS_SAMPLES; ++i) {
        float input = selfOsc ? 0.0f : (phase * 2.0f - 1.0f);  // Sawtooth or silence
        float output = filter.process(input);
        file << i << "," << input << "," << output << "\n";
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
    file.close();
}

int main() {
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "         MS-20 FILTER ANALYSIS - Monotribe Clone Project\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    
    // Test 1: Self-oscillation behavior at various cutoff/resonance settings
    std::cout << "▶ TEST 1: Self-Oscillation Analysis\n";
    std::cout << "Expected: Self-oscillation should occur at resonance > 0.75\n";
    std::cout << "Expected: Frequency should track cutoff parameter\n";
    std::cout << "-----------------------------------------------------------\n";
    
    std::vector<float> cutoffs = {0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f};
    std::vector<float> resonances = {0.0f, 0.5f, 0.75f, 0.85f, 0.95f, 1.0f};
    
    for (float reso : resonances) {
        std::cout << "\nResonance = " << reso << ":\n";
        for (float cutoff : cutoffs) {
            auto result = analyzeSelfOscillation(cutoff, reso);
            std::cout << "  Cutoff " << cutoff << ": ";
            if (result.selfOscillating) {
                std::cout << "✓ Self-osc @ " << result.dominantFreq << "Hz, ";
                std::cout << "RMS=" << result.rmsLevel << ", ";
                std::cout << "Harmonic=" << result.harmonicContent;
            } else {
                std::cout << "✗ No self-oscillation (RMS=" << result.rmsLevel << ")";
            }
            std::cout << "\n";
        }
    }
    
    // Test 2: Impulse response characteristics
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "▶ TEST 2: Impulse Response Analysis\n";
    std::cout << "Expected: Higher resonance = more ringing, longer decay\n";
    std::cout << "Expected: Cutoff should affect decay frequency\n";
    std::cout << "-----------------------------------------------------------\n";
    
    for (float cutoff : {0.3f, 0.5f, 0.7f, 0.9f}) {
        std::cout << "\nCutoff = " << cutoff << ":\n";
        for (float reso : {0.0f, 0.5f, 0.8f, 1.0f}) {
            auto result = analyzeImpulseResponse(cutoff, reso);
            printAnalysis("  ", result);
        }
    }
    
    // Test 3: Continuous signal filtering
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "▶ TEST 3: Continuous Signal (100Hz Sawtooth) Filtering\n";
    std::cout << "Expected: Output level should vary smoothly with cutoff\n";
    std::cout << "Expected: High resonance should add 'bite' to the sound\n";
    std::cout << "-----------------------------------------------------------\n";
    
    for (float reso : {0.0f, 0.5f, 0.8f, 1.0f}) {
        std::cout << "\nResonance = " << reso << ":\n";
        for (float cutoff = 0.1f; cutoff <= 1.0f; cutoff += 0.1f) {
            auto result = analyzeContinuousSignal(cutoff, reso);
            std::cout << "  Cutoff " << std::setw(3) << cutoff 
                      << ": RMS=" << std::setw(6) << result.rmsLevel 
                      << " Peak=" << std::setw(6) << result.peakLevel
                      << " DC=" << std::setw(7) << result.dcOffset << "\n";
        }
    }
    
    // Test 4: Identify issues with current implementation
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "▶ TEST 4: Identified Issues Analysis\n";
    std::cout << "-----------------------------------------------------------\n";
    
    // Issue check: Cutoff below 0.3 should still pass some signal
    {
        MS20Filter filter;
        filter.setSampleRate(SAMPLE_RATE);
        filter.setCutoff(0.25f);
        filter.setResonance(0.3f);
        filter.setActive(true);
        
        float phase = 0;
        float phaseInc = 50.0f / SAMPLE_RATE;
        float maxOutput = 0;
        
        for (int i = 0; i < 44100; ++i) {
            float input = phase * 2.0f - 1.0f;
            float output = filter.process(input);
            maxOutput = std::max(maxOutput, std::abs(output));
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }
        
        std::cout << "Issue 1 - Low cutoff (<0.3) signal: Peak output = " << maxOutput << "\n";
        if (maxOutput < 0.01f) {
            std::cout << "  ⚠ PROBLEM: Signal is being muted at low cutoff values!\n";
            std::cout << "  Real MS-20 should still pass low frequencies.\n";
        } else {
            std::cout << "  ✓ Low frequencies pass through correctly.\n";
        }
    }
    
    // Issue check: DC offset at various settings
    {
        std::cout << "\nIssue 2 - DC Offset analysis:\n";
        for (float cutoff : {0.3f, 0.5f, 0.7f}) {
            for (float reso : {0.5f, 0.8f, 1.0f}) {
                MS20Filter filter;
                filter.setSampleRate(SAMPLE_RATE);
                filter.setCutoff(cutoff);
                filter.setResonance(reso);
                filter.setActive(true);
                
                float phase = 0;
                float phaseInc = 100.0f / SAMPLE_RATE;
                float dcSum = 0;
                
                for (int i = 0; i < 4410; ++i) {
                    filter.process(phase * 2.0f - 1.0f);
                    phase += phaseInc;
                    if (phase >= 1.0f) phase -= 1.0f;
                }
                
                for (int i = 0; i < 4410; ++i) {
                    float input = phase * 2.0f - 1.0f;
                    dcSum += filter.process(input);
                    phase += phaseInc;
                    if (phase >= 1.0f) phase -= 1.0f;
                }
                
                float dc = dcSum / 4410.0f;
                std::cout << "  Cutoff=" << cutoff << " Reso=" << reso << " DC=" << dc;
                if (std::abs(dc) > 0.1f) {
                    std::cout << " ⚠ HIGH DC OFFSET";
                }
                std::cout << "\n";
            }
        }
    }
    
    // Issue check: Self-oscillation frequency tracking
    {
        std::cout << "\nIssue 3 - Self-oscillation frequency tracking:\n";
        std::cout << "  Expected: Frequency should increase with cutoff parameter\n";
        
        float lastFreq = 0;
        bool trackingOK = true;
        
        for (float cutoff : {0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f}) {
            auto result = analyzeSelfOscillation(cutoff, 0.95f);
            std::cout << "  Cutoff " << cutoff << ": " << result.dominantFreq << "Hz";
            if (lastFreq > 0 && result.dominantFreq < lastFreq) {
                std::cout << " ⚠ NOT TRACKING";
                trackingOK = false;
            }
            std::cout << "\n";
            lastFreq = result.dominantFreq;
        }
        
        if (trackingOK) {
            std::cout << "  ✓ Frequency tracking is correct.\n";
        }
    }
    
    // Generate waveform files for visual inspection
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "▶ Generating waveform data files...\n";
    
    writeWaveform("waveform_c50_r00.csv", 0.5f, 0.0f);
    writeWaveform("waveform_c50_r50.csv", 0.5f, 0.5f);
    writeWaveform("waveform_c50_r80.csv", 0.5f, 0.8f);
    writeWaveform("waveform_c50_r100.csv", 0.5f, 1.0f);
    writeWaveform("waveform_c70_r95_selfOsc.csv", 0.7f, 0.95f, true);
    
    std::cout << "  Generated 5 waveform CSV files.\n";
    
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    ANALYSIS COMPLETE\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    
    return 0;
}
