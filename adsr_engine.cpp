#include "adsr_engine.h"
#include <algorithm>

void ADSR::noteOn() {
    state = Attack;
}

void ADSR::noteOff() {
    if (state != Idle) {
        releaseStart = value;
        state = Release;
    }
}

void ADSR::reset() {
    state = Idle;
    value = 0.0f;
    releaseStart = 0.0f;
}

void ADSR::setParams(float a, float d, float s, float r) {
    attack = std::max(a, 0.001f);
    decay = std::max(d, 0.001f);
    sustain = std::clamp(s, 0.0f, 1.0f);
    release = std::max(r, 0.001f);
}

float ADSR::process() {
    switch (state) {
        case Idle:
            break;

        case Attack:
            value += 1.0f / (attack * sampleRate);
            if (value >= 1.0f) {
                value = 1.0f;
                state = Decay;
            }
            break;

        case Decay:
            value -= (1.0f - sustain) / (decay * sampleRate);
            if (value <= sustain) {
                value = sustain;
                state = Sustain;
            }
            break;

        case Sustain:
            // Hold level
            break;

        case Release:
            value -= releaseStart / (release * sampleRate);
            if (value <= 0.0f) {
                value = 0.0f;
                state = Idle;
            }
            break;
    }

    return value;
}
