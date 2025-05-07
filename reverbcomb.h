// reverb.h
#ifndef REVERB_H
#define REVERB_H

#include <algorithm>

class Reverb {
public:
    explicit Reverb(double sampleRate = 44100.0);
    ~Reverb();

    // External API (unchanged)
    void setDecay(float seconds);   // 0.2–5.0 sec mapped to feedback
    void setDamping(float value);   // 0–1 (lowpass in combs)
    void setMix(float value);       // 0–1 dry/wet mix
    void setPreDelay(float ms);     // pre-delay in milliseconds
    float process(float input);     // process one sample

private:
    static const int NUM_COMBS = 8;
    float* combBuffers[NUM_COMBS];
    int combSizes[NUM_COMBS];
    int combIndex[NUM_COMBS];
    float combLastOut[NUM_COMBS];   // one-pole filter state for each comb

    float feedback;    // comb feedback gain (controls decay time)
    float damping;     // one-pole damping coefficient
    float mix;         // dry/wet mix
    double sampleRate;

    float* preDelayBuffer;
    int preDelaySamples;
    int preDelayWriteIndex;

    // All-pass diffuser blocks (series)
    class Allpass {
    public:
        Allpass(int sizeSamples, float feedbackVal)
            : bufferSize(sizeSamples),
              buffer(new float[sizeSamples]()),
              writeIdx(0),
              fb(feedbackVal) {}
        ~Allpass() { delete[] buffer; }

        float process(float input) {
            // Standard all-pass: output = buffer[next] - input, then write input+fb*bufout
            int readIdx = (writeIdx + 1) % bufferSize;
            float bufOut = buffer[readIdx];
            float output = bufOut - input;
            buffer[writeIdx] = input + bufOut * fb;
            writeIdx = (writeIdx + 1) % bufferSize;
            return output;
        }

    private:
        int bufferSize;
        float* buffer;
        int writeIdx;
        float fb;
    };
    // commented out.
    Allpass diffusion1;  // small allpass delays for diffusion
    Allpass diffusion2;
    Allpass diffusion3;
    
};

#endif // REVERB_H
