// lfo_engine.h
#pragma once
#include <cmath>

void ApplyLFOPitch(double& frequency);
void ApplyLFOAmplitude(float& drySample);
void ApplyLFOFilter(float& drySample);


enum class LFOWaveform {
    Sine,
    Square,
    Triangle
};

struct LFO {
    float rateHz = 2.0f;     // LFO frequency in Hz
    float depth = 0.5f;       // Modulation depth
    LFOWaveform waveform = LFOWaveform::Sine;
    float phase = 0.0f;       // Normalized phase [0.0 - 1.0]

    void reset() { phase = 0.0f; }

    float sample(float dt) {
        phase += dt * rateHz;
        if (phase >= 1.0f) phase -= 1.0f;

        float v = 0.0f;
        switch (waveform) {
            case LFOWaveform::Sine:
                v = std::sin(phase * 2.0f * M_PI);
                break;
            case LFOWaveform::Square:
                v = (phase < 0.5f) ? 1.0f : -1.0f;
                break;
            case LFOWaveform::Triangle:
                v = 4.0f * std::abs(phase - 0.5f) - 1.0f;
                break;
        }
        return v * depth;
    }
};

extern LFO lfo;  // Add this declaration
