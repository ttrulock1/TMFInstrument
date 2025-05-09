#pragma once

class MoogFilter {
public:
    MoogFilter(float sampleRate);
    void reset();
    void setCutoff(float cutoffHz);
    void setResonance(float res);
    float process(float input);
    float process(float input, float& highpassOut);

private:
    float sampleRate;
    float cutoff = 1000.0f;
    float resonance = 0.0f;
    float p, k, t1, t2, r;
    float y1, y2, y3, y4;
    float lastHighpass;
};
