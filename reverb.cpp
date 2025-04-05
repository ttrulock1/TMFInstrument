#include "reverb.h"
#include <algorithm> // For std::clamp

Reverb::Reverb(double sampleRate)
    : delays{ DelayLine(149, sampleRate),  // Now using sampleRate
             DelayLine(211, sampleRate),
             DelayLine(263, sampleRate),
             DelayLine(307, sampleRate) },
      feedback(0.7f),
      damping(0.5f),
      mix(0.3f),
      sampleRate(sampleRate), // Initialize sampleRate
      decayAmount(0.7f) {      // Initialize decayAmount
}

void Reverb::setDecay(float seconds) {
    float norm = std::clamp((seconds - 0.2f) / (5.0f - 0.2f), 0.0f, 1.0f);
    decayAmount = 0.4f + norm * 0.55f;
}

// Rest of functions remain the same...

void Reverb::setDamping(float d) {
    damping = std::clamp(d, 0.0f, 1.0f);
}

void Reverb::setMix(float m) {
    mix = std::clamp(m, 0.0f, 1.0f);
}

float Reverb::dampedSample(float input, float& last) {
    float a = damping;
    last += a * (input - last);
    return last;
}

float Reverb::process(float input) {
    float wet = 0.0f;

    for (int i = 0; i < 4; ++i) {
        float delayed = delays[i].read();
        float damped = dampedSample(delayed, lastOutput[i]);
        float fb = damped * decayAmount;

        delays[i].write(input + fb);
        wet += delayed;
    }

    wet *= 0.25f;
    return input * (1.0f - mix) + wet * mix;
}
