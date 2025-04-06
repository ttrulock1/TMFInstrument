// chorus.h
#pragma once

class Chorus {
public:
    Chorus(double sampleRate = 44100.0);
    void setRate(float hz);      // LFO rate in Hz
    void setDepth(float amount); // 0.0 to 1.0 depth scaling
    void setMix(float amount);   // Wet/dry mix
    float process(float input);  // Process one sample

private:
    float sampleRate;

    float lfoPhase = 0.0f;
    float lfoRate = 1.0f;    // Hz
    float depth = 0.5f;      // 0.0 to 1.0
    float mix = 0.5f;        // 0.0 to 1.0

    static constexpr int delayBufferSize = 2048;
    float delayBuffer[delayBufferSize];
    int writeIndex = 0;

    float readFromDelay(float delaySamples);
};
