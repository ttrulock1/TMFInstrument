#ifndef ADSR_ENGINE_H
#define ADSR_ENGINE_H

struct ADSR {
    float attack  = 0.01f;
    float decay   = 0.1f;
    float sustain = 0.8f;
    float release = 0.2f;

    float sampleRate = 44100.0f;

    enum State { Idle, Attack, Decay, Sustain, Release };
    State state = Idle;

    float value = 0.0f;
    float releaseStart = 0.0f;

    void noteOn();
    void noteOff();
    void reset();
    void setParams(float a, float d, float s, float r);
    float process();
};

#endif
