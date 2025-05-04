#pragma once

#include <cmath>

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
    float lfoPhase2 = 0.25f;  // second voice, phase offset (90°)
    float lfoPhase3 = 0.5f;   // third voice, phase offset (180°)

    float lfoRate = 1.0f;     // main LFO rate (Hz)
    float lfoRate2 = 1.01f;   // slight detune for second voice
    float lfoRate3 = 0.99f;   // slight detune for third voice

    float depth = 0.5f;       // 0.0 to 1.0
    float mix = 0.5f;         // 0.0 to 1.0

    static constexpr int delayBufferSize = 2048;
    float delayBuffer[delayBufferSize];
    int writeIndex = 0;

    float readFromDelay(float delaySamples);
};
