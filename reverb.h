// reverb.h
#ifndef REVERB_H
#define REVERB_H

class Reverb {
public:
    explicit Reverb(double sampleRate = 44100.0);
    void setDecay(float value);
    void setDamping(float value);
    void setMix(float value);
    float process(float input);
    
private:
    // In reverb.h
    float dampedSample(float input, float& last); 
    static const int NUM_DELAYS = 4;
    
    struct DelayLine {
        explicit DelayLine(int sizeMs, double sr) {
            length = static_cast<int>((sizeMs / 1000.0) * sr);
            buffer = new float[length]();
            writeIndex = 0;
        }
        
        ~DelayLine() { delete[] buffer; }
        
        void write(float sample) {
            buffer[writeIndex] = sample;
            writeIndex = (writeIndex + 1) % length;
        }
        
        float read() const { // 👈 No parameters here!
            int readIdx = (writeIndex - 1 + length) % length;
            return buffer[readIdx];
        }
        
        float* buffer;
        int length; // 👈 This is the correct member name
        int writeIndex;
    };
    
    DelayLine delays[NUM_DELAYS];
    float lastOutput[NUM_DELAYS] = {0}; // 👈 Missing array
    float feedback;
    float damping;
    float mix;
    double sampleRate;
    float decayAmount;

    //added later.s    // 
    float lfoPhase = 0.0f;

    float lfoPhase1 = lfoPhase;
    float lfoPhase2 = lfoPhase + 0.25f;  // 90° offset
    float lfoPhase3 = lfoPhase + 0.5f;   // 180° offset
    float lfoPhase4 = lfoPhase + 0.75f;  // 270° offset
    float lfoRate = 0.1f;    // very slow, ~0.1 Hz
    float lfoRate1 = 0.1f;
    float lfoRate2 = 0.1f * 1.01f;
    float lfoRate3 = 0.1f * 0.99f;
    float lfoRate4 = 0.1f * 1.02f;


    float lfoAmount = 10.0f;  // modulate ±1.5 samples
};

#endif // REVERB_H