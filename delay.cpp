#include "delay.h"
#include <algorithm> // for std::clamp

Delay::Delay(double sampleRate, int maxDelayMs)
    : sampleRate(sampleRate)
{
    int maxSamples = static_cast<int>((maxDelayMs / 1000.0) * sampleRate);
    buffer.resize(maxSamples, 0.0f);
    writeIndex = 0;
    setDelayTime(250); // default 250ms
}

void Delay::setDelayTime(int ms) {
    int samples = static_cast<int>((ms / 1000.0) * sampleRate);
    delaySamples = std::min(samples, static_cast<int>(buffer.size() - 1));
}

void Delay::setFeedback(float value) {
    feedback = std::clamp(value, 0.0f, 0.99f); // prevent runaway
}

void Delay::setMix(float value) {
    mix = std::clamp(value, 0.0f, 1.0f); // 0 = dry, 1 = wet
}

float Delay::process(float input) {
    int readIndex = writeIndex - delaySamples;
    if (readIndex < 0)
        readIndex += buffer.size();

    float delayed = buffer[readIndex];

    // COMMENT OUT IF DOESN"T WORK.
    float feedbackSample = delayed * feedback;  // 🍀 add this declaration!

    if (mode == DelayMode::Analog) {
        // 🍀 Low-pass filter on feedback
        lpFiltered += lpAlpha * (feedbackSample - lpFiltered);
        feedbackSample = lpFiltered;

        // 🍀 Soft clipping
        feedbackSample = std::tanh(feedbackSample);

    }

    buffer[writeIndex] = input + delayed * feedback;

    writeIndex = (writeIndex + 1) % buffer.size();
    return input * (1.0f - mix) + delayed * mix;
}
