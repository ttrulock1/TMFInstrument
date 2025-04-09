#ifndef SOUND_H
#define SOUND_H

#include <cmath>

#define TWOPI 6.28318530718
const double SAMPLE_RATE = 44100.0;

namespace sound {

    enum WaveType { SINE, SQUARE, SAWTOOTH, TRIANGLE };

    // struct ADSR {
    //     double attack;   // Time in seconds
    //     double decay;    // Time in seconds
    //     double sustain;  // Amplitude (0.0 - 1.0)
    //     double release;  // Time in seconds
    // };

    // inline double ApplyADSR(double time, double duration, ADSR env) {
    //     if (time < env.attack) {
    //         return time / env.attack;
    //     } else if (time < env.attack + env.decay) {
    //         return 1.0 - ((time - env.attack) / env.decay) * (1.0 - env.sustain);
    //     } else if (time < duration - env.release) {
    //         return env.sustain;
    //     } else {
    //         double releaseTime = time - (duration - env.release);
    //         return env.sustain * (1.0 - (releaseTime / env.release));
    //     }
    // }

inline short GenerateWave(WaveType type, double time, double frequency, double amp) {
    double cycles = time * frequency;
    double rad = TWOPI * cycles;
    short amplitude = static_cast<short>(32767 * amp);

    switch (type) {
        case SINE:
            return static_cast<short>(amplitude * std::sin(rad));
        case SQUARE:
            return (std::sin(rad) >= 0) ? amplitude : -amplitude;
        case SAWTOOTH:
            return static_cast<short>(amplitude * (2.0 * (cycles - std::floor(cycles)) - 1.0));
        case TRIANGLE:
            return static_cast<short>(amplitude * (2.0 * std::fabs(2.0 * (cycles - std::floor(cycles + 0.5))) - 1.0));
        default:
            return 0;
    }
}
}

#endif // SOUND_H
