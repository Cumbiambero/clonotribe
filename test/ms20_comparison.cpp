/**
 * MS-20 Filter Comparison Test
 * Compares the original and improved MS-20 filter implementations
 */

#include <algorithm>
#include "../src/dsp/vcf/ms20.hpp"
#include "../src/dsp/vcf/ms20_improved.hpp"
#include "../src/constants.hpp"
#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <iomanip>

using namespace clonotribe;

constexpr float SAMPLE_RATE = 44100.0f;
constexpr int SAMPLES = 8192;

struct TestResult {
    float cutoff;
    float resonance;
    float origRMS;
    float origPeak;
    float origDC;
    bool origSelfOsc;
    float impRMS;
    float impPeak;
    float impDC;
    bool impSelfOsc;
};

float calculateRMS(const std::vector<float>& samples) {
    float sum = 0;
    for (float s : samples) sum += s * s;
    return std::sqrt(sum / samples.size());
}

float calculateDC(const std::vector<float>& samples) {
    float sum = 0;
    for (float s : samples) sum += s;
    return sum / samples.size();
}

float calculatePeak(const std::vector<float>& samples) {
    float peak = 0;
    for (float s : samples) peak = std::max(peak, std::abs(s));
    return peak;
}

TestResult compareSelfOscillation(float cutoff, float resonance) {
    TestResult result;
    result.cutoff = cutoff;
    result.resonance = resonance;
    
    // Test original filter
    {
        MS20Filter filter;
        filter.setSampleRate(SAMPLE_RATE);
        filter.setCutoff(cutoff);
        filter.setResonance(resonance);
        filter.setActive(true);
        
        // Warm up
        for (int i = 0; i < 1000; ++i) filter.process(ZERO);
        
        std::vector<float> samples(SAMPLES);
        for (int i = 0; i < SAMPLES; ++i) samples[i] = filter.process(ZERO);
        
        result.origRMS = calculateRMS(samples);
        result.origPeak = calculatePeak(samples);
        result.origDC = calculateDC(samples);
        result.origSelfOsc = result.origRMS > 0.01f;
    }
    
    // Test improved filter
    {
        MS20FilterImproved filter;
        filter.setSampleRate(SAMPLE_RATE);
        filter.setCutoff(cutoff);
        filter.setResonance(resonance);
        filter.setActive(true);
        
        // Warm up
        for (int i = 0; i < 1000; ++i) filter.process(ZERO);
        
        std::vector<float> samples(SAMPLES);
        for (int i = 0; i < SAMPLES; ++i) samples[i] = filter.process(ZERO);
        
        result.impRMS = calculateRMS(samples);
        result.impPeak = calculatePeak(samples);
        result.impDC = calculateDC(samples);
        result.impSelfOsc = result.impRMS > 0.01f;
    }
    
    return result;
}

TestResult compareSawtoothFiltering(float cutoff, float resonance) {
    TestResult result;
    result.cutoff = cutoff;
    result.resonance = resonance;
    
    float phase = 0;
    float phaseInc = 100.0f / SAMPLE_RATE;  // 100Hz sawtooth
    
    // Test original filter
    {
        MS20Filter filter;
        filter.setSampleRate(SAMPLE_RATE);
        filter.setCutoff(cutoff);
        filter.setResonance(resonance);
        filter.setActive(true);
        phase = 0;
        
        // Warm up
        for (int i = 0; i < 4410; ++i) {
            float input = phase * 2.0f - 1.0f;
            (void)filter.process(input);
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }
        
        std::vector<float> samples(SAMPLES);
        for (int i = 0; i < SAMPLES; ++i) {
            float input = phase * 2.0f - 1.0f;
            samples[i] = filter.process(input);
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }
        
        result.origRMS = calculateRMS(samples);
        result.origPeak = calculatePeak(samples);
        result.origDC = calculateDC(samples);
        result.origSelfOsc = false;
    }
    
    // Test improved filter
    {
        MS20FilterImproved filter;
        filter.setSampleRate(SAMPLE_RATE);
        filter.setCutoff(cutoff);
        filter.setResonance(resonance);
        filter.setActive(true);
        phase = 0;
        
        // Warm up
        for (int i = 0; i < 4410; ++i) {
            float input = phase * 2.0f - 1.0f;
            (void)filter.process(input);
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }
        
        std::vector<float> samples(SAMPLES);
        for (int i = 0; i < SAMPLES; ++i) {
            float input = phase * 2.0f - 1.0f;
            samples[i] = filter.process(input);
            phase += phaseInc;
            if (phase >= 1.0f) phase -= 1.0f;
        }
        
        result.impRMS = calculateRMS(samples);
        result.impPeak = calculatePeak(samples);
        result.impDC = calculateDC(samples);
        result.impSelfOsc = false;
    }
    
    return result;
}

void writeWaveformComparison(const std::string& filename, float cutoff, float resonance) {
    MS20Filter origFilter;
    MS20FilterImproved impFilter;
    
    origFilter.setSampleRate(SAMPLE_RATE);
    origFilter.setCutoff(cutoff);
    origFilter.setResonance(resonance);
    origFilter.setActive(true);
    
    impFilter.setSampleRate(SAMPLE_RATE);
    impFilter.setCutoff(cutoff);
    impFilter.setResonance(resonance);
    impFilter.setActive(true);
    
    std::ofstream file(filename);
    file << "# Comparison: cutoff=" << cutoff << " resonance=" << resonance << "\n";
    file << "# Sample,Input,Original,Improved\n";
    
    float phase = 0;
    float phaseInc = 200.0f / SAMPLE_RATE;
    
    for (int i = 0; i < SAMPLES; ++i) {
        float input = phase * 2.0f - 1.0f;
        float origOut = origFilter.process(input);
        float impOut = impFilter.process(input);
        file << i << "," << input << "," << origOut << "," << impOut << "\n";
        phase += phaseInc;
        if (phase >= 1.0f) phase -= 1.0f;
    }
    file.close();
}

int main() {
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    std::cout << "      MS-20 FILTER COMPARISON: Original vs Improved\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n\n";
    
    std::cout << std::fixed << std::setprecision(4);
    
    // Test 1: Self-oscillation comparison
    std::cout << "▶ TEST 1: Self-Oscillation Comparison\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    std::cout << "Cutoff  Reso    │ Original          │ Improved\n";
    std::cout << "                │ RMS    Peak SelfO │ RMS    Peak SelfO\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    
    std::vector<float> cutoffs = {0.3f, 0.5f, 0.7f, 0.9f};
    std::vector<float> resonances = {0.5f, 0.75f, 0.85f, 0.95f, 1.0f};
    
    for (float reso : resonances) {
        for (float cutoff : cutoffs) {
            auto result = compareSelfOscillation(cutoff, reso);
            std::cout << std::setw(6) << cutoff << "  " << std::setw(4) << reso << "  │ ";
            std::cout << std::setw(6) << result.origRMS << " " << std::setw(5) << result.origPeak << " ";
            std::cout << (result.origSelfOsc ? "YES " : "NO  ") << "│ ";
            std::cout << std::setw(6) << result.impRMS << " " << std::setw(5) << result.impPeak << " ";
            std::cout << (result.impSelfOsc ? "YES" : "NO") << "\n";
        }
        std::cout << "─────────────────────────────────────────────────────────────────\n";
    }
    
    // Test 2: Signal filtering comparison (low cutoff - the problem area)
    std::cout << "\n▶ TEST 2: Low Cutoff Signal Filtering (Problem Area)\n";
    std::cout << "Expected: Improved should pass signal at low cutoff values\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    std::cout << "Cutoff  Reso    │ Original          │ Improved\n";
    std::cout << "                │ RMS    Peak DC    │ RMS    Peak DC\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    
    std::vector<float> lowCutoffs = {0.1f, 0.2f, 0.25f, 0.3f, 0.35f, 0.4f};
    
    for (float cutoff : lowCutoffs) {
        auto result = compareSawtoothFiltering(cutoff, 0.3f);
        std::cout << std::setw(6) << cutoff << "  0.30  │ ";
        std::cout << std::setw(6) << result.origRMS << " " << std::setw(5) << result.origPeak << " ";
        std::cout << std::setw(5) << result.origDC << " │ ";
        std::cout << std::setw(6) << result.impRMS << " " << std::setw(5) << result.impPeak << " ";
        std::cout << std::setw(5) << result.impDC << "\n";
    }
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    
    // Test 3: Full cutoff sweep with different resonances
    std::cout << "\n▶ TEST 3: Full Cutoff Sweep (100Hz Sawtooth Input)\n";
    std::cout << "─────────────────────────────────────────────────────────────────\n";
    
    for (float reso : {0.0f, 0.5f, 0.8f}) {
        std::cout << "\nResonance = " << reso << ":\n";
        std::cout << "Cutoff  │ Original RMS    │ Improved RMS    │ Difference\n";
        std::cout << "────────┼─────────────────┼─────────────────┼───────────\n";
        
        for (float cutoff = 0.1f; cutoff <= 1.0f; cutoff += 0.1f) {
            auto result = compareSawtoothFiltering(cutoff, reso);
            float diff = result.impRMS - result.origRMS;
            std::cout << std::setw(6) << cutoff << "  │ ";
            std::cout << std::setw(15) << result.origRMS << " │ ";
            std::cout << std::setw(15) << result.impRMS << " │ ";
            std::cout << std::setw(9) << diff << "\n";
        }
    }
    
    // Generate comparison waveforms
    std::cout << "\n▶ Generating comparison waveform files...\n";
    writeWaveformComparison("compare_c20_r30.csv", 0.2f, 0.3f);
    writeWaveformComparison("compare_c50_r50.csv", 0.5f, 0.5f);
    writeWaveformComparison("compare_c50_r90.csv", 0.5f, 0.9f);
    writeWaveformComparison("compare_c70_r80.csv", 0.7f, 0.8f);
    std::cout << "  Generated 4 comparison CSV files.\n";
    
    std::cout << "\n═══════════════════════════════════════════════════════════════\n";
    std::cout << "                    COMPARISON COMPLETE\n";
    std::cout << "═══════════════════════════════════════════════════════════════\n";
    
    return 0;
}
