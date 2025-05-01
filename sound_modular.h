// sound_modular.h
#ifndef SOUND_MODULAR_H
#define SOUND_MODULAR_H
#include "shared_buffer.h"


#include <cmath>
#include <algorithm>

#define TWOPI 6.28318530718
constexpr double SAMPLE_RATE = 44100.0;

namespace sound_modular {

struct ADSR {
    float attack;
    float decay;
    float sustain;
    float release;
    float current = 0.0f;
    float time = 0.0f;
    enum State { Attack, Decay, Sustain, Release, Idle } state = Idle;

    void noteOn() {
        state = Attack;
        time = 0.0f;
    }

    void noteOff() {
        state = Release;
        time = 0.0f;
    }

    float process() {
        float dt = 1.0f / SAMPLE_RATE;
        time += dt;
        switch (state) {
            case Attack:
                current += dt / attack;
                if (current >= 1.0f) {
                    current = 1.0f;
                    state = Decay;
                    time = 0.0f;
                }
                break;
            case Decay:
                current -= dt * (1.0f - sustain) / decay;
                if (current <= sustain) {
                    current = sustain;
                    state = Sustain;
                }
                break;
            case Sustain:
                break;
            case Release:
                current -= dt * sustain / release;
                if (current <= 0.0f) {
                    current = 0.0f;
                    state = Idle;
                }
                break;
            case Idle:
                current = 0.0f;
                break;
        }
        return current;
    }
};

inline double GetSine(double time, double freq) {
    return std::sin(TWOPI * freq * time);
}

inline double GetSaw(double time, double freq) {
    double cycles = time * freq;
    return 2.0 * (cycles - std::floor(cycles)) - 1.0;
}

inline double GetSquare(double time, double freq, float duty) {
    return (std::fmod(time * freq, 1.0) < duty) ? 1.0 : -1.0;
}

inline double GetTriangle(double time, double freq) {
    return 2.0 * std::fabs(2.0 * (time * freq - std::floor(time * freq + 0.5))) - 1.0;
}

inline double ApplyUltrasaw(double time, double freq, float amount) {
    double base = GetSaw(time, freq);
    double detune = 0.01 * amount;
    double mod = GetSaw(time, freq + detune) + GetSaw(time, freq - detune);
    return (base + 0.5 * mod) / 2.0;
}

inline double ApplyMetalizer(double tri, float amount) {
    return std::tanh(tri * (1.0 + amount * 5.0));
}

inline double ApplyPWM(double time, double freq, float amount) {
    float duty = 0.5f + 0.45f * amount;
    return GetSquare(time, freq, duty);
}

inline double ApplySubOsc(double time, double freq, float amount) {
    return amount * GetSquare(time, freq / 2.0, 0.5);
}

inline double ApplySaturation(double input, float amount) {
    return std::tanh(input * (1.0 + 4.0 * amount));
}

struct ModularVoice {
    bool active = false;
    double time = 0.0;
    double duration = 0.0;
    double frequency = 440.0;
    ADSR ampEnv;

    float sineLevel = 0.0f;
    float squareLevel = 0.0f;
    float sawLevel = 0.0f;
    float triLevel = 0.0f;
    float subLevel = 0.0f;

    float pwmAmount = 0.0f;
    float metalAmount = 0.0f;
    float ultrasawAmount = 0.0f;
    float saturationAmount = 0.0f;

    float volume = 1.0f;

    short getSample() {
        float amp = ampEnv.process();
        if (ampEnv.state == ADSR::Idle) active = false;
        time += 1.0 / SAMPLE_RATE;

        double sine     = sineLevel * GetSine(time, frequency);
        double square   = squareLevel * ApplyPWM(time, frequency, pwmAmount);
        double saw      = sawLevel * ApplyUltrasaw(time, frequency, ultrasawAmount);
        double triangle = triLevel * ApplyMetalizer(GetTriangle(time, frequency), metalAmount);
        double sub      = subLevel * ApplySubOsc(time, frequency, 1.0f);

        double mixed = (sine + square + saw + triangle + sub);

        mixed = ApplySaturation(mixed, saturationAmount);
        double noise = ((rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.3 * oscNoiseAmount.load();
        mixed += noise;
        mixed *= amp * volume;

        return static_cast<short>(std::clamp(mixed, -1.0, 1.0) * 32767);
    }
};

} // namespace sound_modular

#endif // SOUND_MODULAR_H
