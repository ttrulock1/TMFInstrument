#include "Moog_Filter.h"
#include <cmath>

MoogFilter::MoogFilter(float sampleRate) : sampleRate(sampleRate) { reset(); }

void MoogFilter::reset() {
    y1 = y2 = y3 = y4 = 0.0f;
    lastHighpass = 0.0f;
}

void MoogFilter::setCutoff(float cutoffHz) {
    cutoff = cutoffHz;
    float fc = cutoff / sampleRate;
    fc = (fc > 0.99f) ? 0.99f : (fc < 0.01f ? 0.01f : fc);
    p = fc * (1.8f - 0.8f * fc);
    k = 2.0f * std::sin(fc * 3.1415926f * 0.5f) - 1.0f;
    t1 = (1.0f - p) * 1.386249f;
    t2 = 12.0f + t1 * t1;
    r = resonance * (t2 + 6.0f * t1) / (t2 - 6.0f * t1);
}

void MoogFilter::setResonance(float res) {
    resonance = res;
}

float MoogFilter::process(float input) {
    input -= r * y4;
    float x = input * 0.35013f * (input * input + 1.0f);
    y1 = x * p + y1 * (1 - p);
    y2 = y1 * p + y2 * (1 - p);
    y3 = y2 * p + y3 * (1 - p);
    y4 = y3 * p + y4 * (1 - p);
    lastHighpass = input - y4;  // 🎯 high-pass from input - low-pass
    return y4;
}

float MoogFilter::process(float input, float& highpassOut) {
    float lp = process(input);
    highpassOut = lastHighpass;
    return lp;
}
