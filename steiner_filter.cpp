// steiner_filter.cpp

#include "steiner_filter.h"
#include <cmath>
#include <algorithm>

SteinerFilter::SteinerFilter(double sampleRate)
    : sampleRate(sampleRate), cutoff(1000.0f), resonance(0.0f), vcaLevel(1.0f),
      buf0(0.0f), buf1(0.0f), buf2(0.0f), buf3(0.0f), feedbackAmount(0.0f) {}

void SteinerFilter::setCutoff(float cutoffHz) {
    cutoff = std::clamp(cutoffHz, 20.0f, static_cast<float>(sampleRate / 2.0));
}

void SteinerFilter::setResonance(float res) {
    resonance = std::clamp(res, 0.0f, 1.0f);
}

void SteinerFilter::setVcaLevel(float level) {
    vcaLevel = std::clamp(level, 0.0f, 1.0f);
}

float SteinerFilter::process(float input) {
    // simplified Steiner-Parker low-pass approximation
    float f = 2.0f * std::sin(M_PI * cutoff / sampleRate);
    feedbackAmount = resonance * 4.0f * (1.0f - 0.15f * f * f);

    input -= buf3 * feedbackAmount;
    input *= 0.35013f * (f * f) * (f * f);

    buf0 = input + 0.3f * buf0 + 1e-5f;
    buf1 = buf0 + 0.3f * buf1 + 1e-5f;
    buf2 = buf1 + 0.3f * buf2 + 1e-5f;
    buf3 = buf2 + 0.3f * buf3 + 1e-5f;

    // output after 4-pole filtering
    float output = buf3;

    // apply VCA (amplitude control)
    output *= vcaLevel;

    return output;
}
