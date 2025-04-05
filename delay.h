#ifndef DELAY_H
#define DELAY_H

#include <vector>

class Delay {
public:
    Delay(double sampleRate = 44100.0, int maxDelayMs = 1000);
    void setDelayTime(int ms);
    void setFeedback(float value);
    void setMix(float value);
    float process(float input);

private:
    std::vector<float> buffer;
    int writeIndex = 0;
    int delaySamples = 0;
    float feedback = 0.4f;
    float mix = 0.3f;
    double sampleRate;
};

#endif // DELAY_H
