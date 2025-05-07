#include "reverb.h"
#include <cmath>
#include <algorithm>

Reverb::Reverb(double sr)
    : diffusion1(142, 0.7f), diffusion2(107, 0.7f), diffusion3(379, 0.7f),
      sampleRate(sr)
{
    feedback = 0.8f;
    damping = 0.5f;
    mix = 0.3f;

    // Pre-delay setup (max 1 second buffer)
    preDelayWriteIndex = 0;
    preDelaySamples = 0;
    int bufferLength = static_cast<int>(sampleRate);
    preDelayBuffer = new float[bufferLength]();

    // Initialize comb filters
    double rateRatio = sampleRate / 44100.0;
    int baseLengths[NUM_COMBS] = {1557, 1617, 1491, 1422,
                                  1277, 1356, 1188, 1116};
    for (int i = 0; i < NUM_COMBS; ++i) {
        combSizes[i] = static_cast<int>(baseLengths[i] * rateRatio);
        combBuffers[i] = new float[combSizes[i]]();
        combIndex[i] = 0;
        combLastOut[i] = 0.0f;
    }
}

Reverb::~Reverb() {
    for (int i = 0; i < NUM_COMBS; ++i) {
        delete[] combBuffers[i];
    }
    delete[] preDelayBuffer;
}

void Reverb::setDecay(float seconds) {
    float norm = std::clamp((seconds - 0.2f) / (5.0f - 0.2f), 0.0f, 1.0f);
    feedback = 0.4f + norm * 0.55f;
}

void Reverb::setDamping(float d) {
    damping = std::clamp(d, 0.0f, 1.0f);
}

void Reverb::setMix(float m) {
    mix = std::clamp(m, 0.0f, 1.0f);
}

void Reverb::setPreDelay(float ms) {
    int bufferLength = static_cast<int>(sampleRate);
    int samples = static_cast<int>((ms / 1000.0f) * sampleRate);
    preDelaySamples = std::clamp(samples, 0, bufferLength);
}

float Reverb::process(float input) {
    float in = input;
    if (preDelaySamples > 0) {
        int bufferLength = static_cast<int>(sampleRate);
        int readIndex = (preDelayWriteIndex < preDelaySamples)
                        ? preDelayWriteIndex + bufferLength - preDelaySamples
                        : preDelayWriteIndex - preDelaySamples;
        in = preDelayBuffer[readIndex];
        preDelayBuffer[preDelayWriteIndex] = input;
        preDelayWriteIndex = (preDelayWriteIndex + 1) % bufferLength;
    }

    float combSum = 0.0f;
    for (int i = 0; i < NUM_COMBS; ++i) {
        int idx = combIndex[i];
        float out = combBuffers[i][idx];
        combLastOut[i] += (out - combLastOut[i]) * damping;
        combBuffers[i][idx] = in + combLastOut[i] * feedback;
        combIndex[i] = (idx + 1) % combSizes[i];
        combSum += combLastOut[i];
    }
    float combOut = combSum / static_cast<float>(NUM_COMBS);

    float diffused = diffusion3.process(
                        diffusion2.process(
                            diffusion1.process(combOut)));

    return input * (1.0f - mix) + diffused * mix;
}
