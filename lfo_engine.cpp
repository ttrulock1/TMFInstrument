#include "lfo_engine.h"
#include "lfo_ui.h"

#include "shared_buffer.h"
#include "sound_modular.h"
#include <cmath>
#include <algorithm> // for std::clamp

void ApplyLFOPitch(double& frequency) {
    if (lfoTargetRouting.load() == 0) {
        float mod = lfo.sample(1.0f / SAMPLE_RATE);
        float semitoneShift = mod * 0.1f; // gentle vibrato
        frequency *= std::pow(2.0, semitoneShift / 12.0);
    }
}

// 🟨 Placeholder for amplitude modulation
void ApplyLFOAmplitude(float& drySample) {
    if (lfoTargetRouting.load() == 1) {
        float mod = lfo.sample(1.0f / SAMPLE_RATE);
        drySample *= 1.0f + (mod * 0.5f);
    }
}

// 🟪 Placeholder for filter modulation
void ApplyLFOFilter(float& drySample) {
    if (lfoTargetRouting.load() == 2) {
        float mod = lfo.sample(1.0f / SAMPLE_RATE);
        static float filterState = 0.0f;
        float cutoff = 0.05f + 0.45f * ((mod + 1.0f) / 2.0f);
        filterState += (drySample - filterState) * cutoff;
        drySample = filterState;
    }
}
