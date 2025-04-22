#include "effects_backend.h"
#include "shared_buffer.h"   // Globals: delayTime, delayFeedback, delayMix, chorusRate, chorusDepth, chorusMix, reverbDecay, reverbDamping, reverbMix, etc.
#include <algorithm>
#include <cmath>

// Constructor: create effect objects with appropriate sample rate.
EffectsBackend::EffectsBackend()
    : delayEffect(SAMPLE_RATE, 1000),  // 1 sec buffer, adjust as needed
      reverbEffect(44100.0),            // using same sample rate
      chorusEffect(44100.0)
{
    updateParameters();
}

EffectsBackend::~EffectsBackend() {
    // Effects cleanup if needed (assume destructors handle their own cleanup)
}

void EffectsBackend::updateParameters() {
    // Update delay parameters:
    delayEffect.setDelayTime(static_cast<int>(delayTime.load())); // assuming delayTime is in ms or desired units
    delayEffect.setFeedback(delayFeedback.load());
    delayEffect.setMix(delayMix.load());
    // Update chorus parameters:
    chorusEffect.setRate(chorusRate.load());
    chorusEffect.setDepth(chorusDepth.load());
    chorusEffect.setMix(chorusMix.load());
    // Update reverb parameters:
    reverbEffect.setDecay(reverbDecay.load());
    reverbEffect.setDamping(reverbDamping.load());
    reverbEffect.setMix(reverbMix.load());
}

float EffectsBackend::processEffects(float input) {
    float sample = input;
    // Process effects in series: Chorus -> Delay -> Reverb

    if (chorusEnabled.load()) {
        sample = chorusEffect.process(sample);
    }
    if (delayEnabled.load()) {
        // Update delay parameters every call if desired
        delayEffect.setDelayTime(static_cast<int>(delayTime.load()));
        delayEffect.setFeedback(delayFeedback.load());
        delayEffect.setMix(delayMix.load());
        sample = delayEffect.process(sample);
    }
    if (reverbEnabled.load()) {
        reverbEffect.setDecay(reverbDecay.load());
        reverbEffect.setDamping(reverbDamping.load());
        reverbEffect.setMix(reverbMix.load());
        sample = reverbEffect.process(sample);
    }
    return sample;
}
