// chorus.cpp
#include "chorus.h"
#include <cmath>
#include <algorithm>  // 👈 for std::clamp


Chorus::Chorus(double sr) : sampleRate(sr) {
    for (int i = 0; i < delayBufferSize; ++i)
        delayBuffer[i] = 0.0f;
}

void Chorus::setRate(float hz) {
    lfoRate = hz;
}

void Chorus::setDepth(float amount) {
    depth = std::clamp(amount, 0.0f, 1.0f);
}

void Chorus::setMix(float amount) {
    mix = std::clamp(amount, 0.0f, 1.0f);
}

float Chorus::readFromDelay(float delaySamples) {
    float readIndex = static_cast<float>(writeIndex) - delaySamples;
    if (readIndex < 0) readIndex += delayBufferSize;

    int index1 = static_cast<int>(readIndex) % delayBufferSize;
    int index2 = (index1 + 1) % delayBufferSize;
    float frac = readIndex - std::floor(readIndex);

    // Linear interpolation
    return delayBuffer[index1] * (1.0f - frac) + delayBuffer[index2] * frac;
}

float Chorus::process(float input) {
    // Delay modulated by sine LFO
    float lfo = std::sin(2.0f * M_PI * lfoPhase);
    lfoPhase += lfoRate / sampleRate;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

    float modDelayMs = 5.0f + 10.0f * depth * (lfo + 1.0f) * 0.5f; // 5–15ms
    float modDelaySamples = modDelayMs * 0.001f * sampleRate;

    float delayed = readFromDelay(modDelaySamples);

    delayBuffer[writeIndex] = input;
    writeIndex = (writeIndex + 1) % delayBufferSize;

    return (1.0f - mix) * input + mix * delayed;
}
