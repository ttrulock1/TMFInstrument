#ifndef DELAY_H
#define DELAY_H

#include <vector>
#include <cmath> // 🍀 comment out if doesn't work for std::tanh (if using tanh saturation)
// 🍀 Analog mode control commen out
enum class DelayMode { Digital, Analog }; // 🍀


class Delay {
public:
    Delay(double sampleRate = 44100.0, int maxDelayMs = 1000);
    void setDelayTime(int ms);
    void setFeedback(float value);
    void setMix(float value);
    float process(float input);

//comment out if doesn't work
    void setMode(DelayMode newMode) { mode = newMode; } // 🍀

private:
    std::vector<float> buffer;
    int writeIndex = 0;
    int delaySamples = 0;
    float feedback = 0.4f;
    float mix = 0.3f;
    double sampleRate;

       // 🍀 Analog mode state: comment out if doesn't work.
    DelayMode mode = DelayMode::Digital; // 🍀 default to Digital
    float lpFiltered = 0.8f;  // 🍀 low-pass filter state
    float lpAlpha = 0.9f;     // 🍀 low-pass filter strength (adjust as needed)
};

#endif // DELAY_H
