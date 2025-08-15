#pragma once
#include <vector>
#include <cmath>

namespace clonotribe {

class Delay {
public:
    Delay() {
        setSampleRate(44100.0f);
        setMaxDelayTime(2.0f);
    }
    
    void setSampleRate(float sampleRate) {
        this->sampleRate = sampleRate;
        if (maxDelaySamples > 0) {
            setMaxDelayTime(maxDelayTime);
        }
    }
    
    void setMaxDelayTime(float maxTime) {
        maxDelayTime = maxTime;
        maxDelaySamples = static_cast<int>(maxTime * sampleRate) + 1;
        buffer.resize(maxDelaySamples, 0.0f);
        writeIndex = 0;
    }
    
    float process(float input, float clockTrigger, float time, float amount) {
        if (buffer.empty()) return input;
        
        amount = std::clamp(amount, 0.0f, 1.0f);
        if (amount <= 0.0f) return input;
        
        input = std::clamp(input, -10.0f, 10.0f);
        
        bool clockTriggered = clockTrigger > 1.0f && lastClockTrigger <= 1.0f;
        lastClockTrigger = clockTrigger;
        
        float delayTime;
        
        if (clockTriggered) {
            float measuredTime = static_cast<float>(samplesSinceLastClock) / sampleRate;
            if (measuredTime > 0.01f && measuredTime < 4.0f) {
                lastClockInterval = measuredTime;
            }
            samplesSinceLastClock = 0;
        } else {
            samplesSinceLastClock++;
        }
        
        if (clockTrigger > 0.1f && static_cast<float>(samplesSinceLastClock) < sampleRate * 2.0f && lastClockInterval > 0.0f) {
            delayTime = lastClockInterval;
        } else {
            delayTime = 0.01f + time * 1.99f;
        }
        
        delayTime = std::clamp(delayTime, 0.001f, maxDelayTime);
        float targetDelaySamples = delayTime * sampleRate;
        targetDelaySamples = std::clamp(targetDelaySamples, 1.0f, static_cast<float>(maxDelaySamples - 1));
        smoothedDelaySamples += (targetDelaySamples - smoothedDelaySamples) * 0.01f;
        int delaySamples = static_cast<int>(smoothedDelaySamples);
        float fraction = smoothedDelaySamples - static_cast<float>(delaySamples);
        int readIndex1 = writeIndex - delaySamples;
        int readIndex2 = writeIndex - delaySamples - 1;
        if (readIndex1 < 0) readIndex1 += maxDelaySamples;
        if (readIndex2 < 0) readIndex2 += maxDelaySamples;
        float sample1 = buffer[static_cast<size_t>(readIndex1)];
        float sample2 = buffer[static_cast<size_t>(readIndex2)];
        float delayedSample = sample1 + fraction * (sample2 - sample1);
        float feedback = amount * 0.4f;
        float feedbackSignal = delayedSample * feedback;
        feedbackSignal = std::clamp(feedbackSignal, -2.0f, 2.0f);
        buffer[writeIndex] = input + feedbackSignal;
        writeIndex = (writeIndex + 1) % maxDelaySamples;
        return input * (1.0f - amount) + delayedSample * amount;
    }
    
    bool isClockConnected() const {
        return static_cast<float>(samplesSinceLastClock) < sampleRate * 2.0f && lastClockInterval > 0.0f;
    }
    
    void clear() {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        lastClockTrigger = 0.0f;
        samplesSinceLastClock = 0;
        lastClockInterval = 0.0f;
        smoothedDelaySamples = 1.0f;
    }
    
private:
    std::vector<float> buffer;
    int maxDelaySamples = 0;
    int writeIndex = 0;
    float maxDelayTime = 2.0f;
    float sampleRate = 44100.0f;
    float lastClockTrigger = 0.0f;
    int samplesSinceLastClock = 0;
    float lastClockInterval = 0.0f;
    float smoothedDelaySamples = 1.0f;
};
}