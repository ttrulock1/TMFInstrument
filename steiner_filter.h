// steiner_filter.h

#pragma once

class SteinerFilter {
public:
    SteinerFilter(double sampleRate);

    void setCutoff(float cutoffHz);
    void setResonance(float resonance);
    void setVcaLevel(float level);

    // Use per-sample resonance and vca
    float process(float input, float resonance, float vca);

private:
    double sampleRate;
    float cutoffHz = 1000.0f;  // renamed to match .cpp
    float resonance;   // resonance 0.0 - 1.0
    float vcaLevel;    // VCA gain 0.0 - 1.0

    float buf0, buf1, buf2, buf3; // filter buffers
    float feedbackAmount;
};
