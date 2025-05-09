// steiner_filter.h

#ifndef STEINER_FILTER_H
#define STEINER_FILTER_H

class SteinerFilter {
public:
    SteinerFilter(double sampleRate);

    void setCutoff(float cutoffHz);
    void setResonance(float resonance);
    void setVcaLevel(float level);

    float process(float input);

private:
    double sampleRate;
    float cutoff;      // cutoff frequency in Hz
    float resonance;   // resonance 0.0 - 1.0
    float vcaLevel;    // VCA gain 0.0 - 1.0

    float buf0, buf1, buf2, buf3; // filter buffers
    float feedbackAmount;
};

#endif // STEINER_FILTER_H
