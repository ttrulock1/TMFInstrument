#ifndef EFFECTS_BACKEND_H
#define EFFECTS_BACKEND_H

#include "sound.h"   // for SAMPLE_RATE
#include <SDL2/SDL.h>

// EffectsBackend encapsulates your effect processing (chorus, delay, reverb)
// It uses your global shared parameters (delayTime, delayFeedback, etc.) declared in shared_buffer.h.
class EffectsBackend {
public:
    EffectsBackend();
    ~EffectsBackend();

    // Call this for each audio sample to process through the effects chain.
    // 'input' is a normalized value (-1.0 to 1.0); returns the processed sample.
    float processEffects(float input);

    // Update all internal effect parameters using current global values.
    void updateParameters();

private:
    // Effect instances. They are constructed with SAMPLE_RATE.
    Delay delayEffect;
    Reverb reverbEffect;
    Chorus chorusEffect;
};

#endif // EFFECTS_BACKEND_H
