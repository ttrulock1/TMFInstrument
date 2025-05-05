#include "reverb.h"
#include <algorithm> // For std::clamp
#include <cmath>     // For sin(), tanh()

Reverb::Reverb(double sampleRate)
    : delays{ DelayLine(149, sampleRate),
             DelayLine(211, sampleRate),
             DelayLine(263, sampleRate),
             DelayLine(307, sampleRate) },
      feedback(0.7f),
      damping(0.5f),
      mix(0.3f),
      sampleRate(sampleRate),
      decayAmount(0.7f),
      lfoPhase(0.0f),
      lfoRate(0.1f),    // ~0.1 Hz, very slow modulation
      lfoAmount(1.5f) { // ±1.5 samples modulation
}

void Reverb::setDecay(float seconds) {
    float norm = std::clamp((seconds - 0.2f) / (5.0f - 0.2f), 0.0f, 1.0f);
    decayAmount = 0.4f + norm * 0.55f;
}

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

    // Update LFO phase (for modulation)
    lfoPhase += lfoRate / sampleRate;
    if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;
    float lfoMod = lfoAmount * std::sin(2.0f * M_PI * lfoPhase);

    for (int i = 0; i < 4; ++i) {
         float lfoMod;
    switch (i) {
        case 0:
            lfoPhase1 += lfoRate1 / sampleRate;
            if (lfoPhase1 >= 1.0f) lfoPhase1 -= 1.0f;
            lfoMod = lfoAmount * std::sin(2.0f * M_PI * lfoPhase1);
            break;
        case 1:
            lfoPhase2 += lfoRate2 / sampleRate;
            if (lfoPhase2 >= 1.0f) lfoPhase2 -= 1.0f;
            lfoMod = lfoAmount * std::sin(2.0f * M_PI * lfoPhase2);
            break;
        case 2:
            lfoPhase3 += lfoRate3 / sampleRate;
            if (lfoPhase3 >= 1.0f) lfoPhase3 -= 1.0f;
            lfoMod = lfoAmount * std::sin(2.0f * M_PI * lfoPhase3);
            break;
        case 3:
            lfoPhase4 += lfoRate4 / sampleRate;
            if (lfoPhase4 >= 1.0f) lfoPhase4 -= 1.0f;
            lfoMod = lfoAmount * std::sin(2.0f * M_PI * lfoPhase4);
            break;
    }

        // Apply modulation: wobble the delay tap slightly
       float readPos = static_cast<float>(delays[i].writeIndex) - 1 + lfoMod;
        if (readPos < 0) readPos += delays[i].length;

        int index1 = static_cast<int>(readPos) % delays[i].length;
        int index2 = (index1 + 1) % delays[i].length;
        float frac = readPos - static_cast<float>(index1);

        // Linear interpolation between the two delay samples
        float delayed = delays[i].buffer[index1] * (1.0f - frac) + delays[i].buffer[index2] * frac;


        float damped = dampedSample(delayed, lastOutput[i]);

        // Apply soft saturation to feedback path
        float saturatedFb = std::tanh(damped * decayAmount);

        float inputGain = 0.3f;
        float feedbackGain = 0.9f;

        delays[i].write((input * inputGain) + (saturatedFb * feedbackGain));

        wet += delayed;
    }

    wet *= 0.25f;
    return input * (1.0f - mix) + wet * mix;
}
