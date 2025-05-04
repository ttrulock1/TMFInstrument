#include "chorus.h"
#include <cmath>
#include <algorithm>  // for std::clamp

Chorus::Chorus(double sr) : sampleRate(sr) {
    for (int i = 0; i < delayBufferSize; ++i)
        delayBuffer[i] = 0.0f;
}

void Chorus::setRate(float hz) {
    lfoRate = std::clamp(hz, 0.6f, 7.0f);
    lfoRate2 = lfoRate * 1.01f;  // slight detune for second voice
    lfoRate3 = lfoRate * 0.99f;  // slight detune for third voice
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
    // First LFO
    float lfo1 = std::sin(2.0f * M_PI * lfoPhase);
    lfoPhase += lfoRate / sampleRate;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

    // Second LFO (phase offset + slight detune)
    float lfo2 = std::sin(2.0f * M_PI * lfoPhase2);
    lfoPhase2 += lfoRate2 / sampleRate;
    if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;

    // Third LFO (new)
    float lfo3 = std::sin(2.0f * M_PI * lfoPhase3);
    lfoPhase3 += lfoRate3 / sampleRate;
    if (lfoPhase3 >= 1.0f) lfoPhase3 -= 1.0f;

    // Delay times
    float modDelayMs1 = 7.0f + 13.0f * depth * (lfo1 + 1.0f) * 0.5f;
    float modDelayMs2 = 7.0f + 13.0f * depth * (lfo2 + 1.0f) * 0.5f;
    float modDelayMs3 = 7.0f + 13.0f * depth * (lfo3 + 1.0f) * 0.5f;

    float modDelaySamples1 = modDelayMs1 * 0.001f * sampleRate;
    float modDelaySamples2 = modDelayMs2 * 0.001f * sampleRate;
    float modDelaySamples3 = modDelayMs3 * 0.001f * sampleRate;

    // Read all delayed taps
    float delayed1 = readFromDelay(modDelaySamples1);
    float delayed2 = readFromDelay(modDelaySamples2);
    float delayed3 = readFromDelay(modDelaySamples3);

    // Write input into buffer
    delayBuffer[writeIndex] = input;
    writeIndex = (writeIndex + 1) % delayBufferSize;

    // Analog-style smoothing: one-pole lowpass
    static float smoothed1 = 0.0f;
    static float smoothed2 = 0.0f;
    static float smoothed3 = 0.0f;
    float alpha = 0.2f;  // adjust: 0.1 darker, 0.3 brighter

    smoothed1 += alpha * (delayed1 - smoothed1);
    smoothed2 += alpha * (delayed2 - smoothed2);
    smoothed3 += alpha * (delayed3 - smoothed3);

    // Combine all voices
    float wetSignal = (smoothed1 + smoothed2 + smoothed3) / 3.0f;

    // Soft saturation (analog warmth)
    wetSignal = 0.8f * std::tanh(1.25f * wetSignal);

    // Final wet/dry mix
    return (1.0f - mix) * input + mix * wetSignal;
}
