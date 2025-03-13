#include <cmath>

#define TWOPI 6.28318530718

namespace sound {
    inline short SineWave(double time, double frequency, double amp) {
        double rad = TWOPI * frequency * time;
        return static_cast<short>(32767 * amp * std::sin(rad));
    }


}

// namespace sound {
//     #define TWOPI 6.28318530718

//     enum WaveType { SINE, SQUARE, SAWTOOTH, TRIANGLE };

//     struct ADSR {
//         double attack;   // Time in seconds
//         double decay;    // Time in seconds
//         double sustain;  // Amplitude (0.0 - 1.0)
//         double release;  // Time in seconds
//     };

//     double ApplyADSR(double time, double duration, ADSR env) {
//         if (time < env.attack) {
//             return time / env.attack;  // Attack phase (0 -> 1)
//         } else if (time < env.attack + env.decay) {
//             return 1.0 - ((time - env.attack) / env.decay) * (1.0 - env.sustain);  // Decay phase
//         } else if (time < duration - env.release) {
//             return env.sustain;  // Sustain phase
//         } else {
//             double releaseTime = time - (duration - env.release);
//             return env.sustain * (1.0 - (releaseTime / env.release));  // Release phase
//         }
//     }

//     short GenerateWave(WaveType type, double time, double duration, double frequency, double amp, ADSR env) {
//         double tpc = 44100.0 / frequency;
//         double cycles = time / tpc;
//         double rad = TWOPI * cycles;
//         short amplitude = static_cast<short>(32767 * amp * ApplyADSR(time, duration, env));

//         switch (type) {
//             case SINE:
//                 return static_cast<short>(amplitude * std::sin(rad));
//             case SQUARE:
//                 return (std::sin(rad) >= 0) ? amplitude : -amplitude;
//             case SAWTOOTH:
//                 return static_cast<short>(amplitude * (2.0 * (cycles - floor(cycles)) - 1.0));
//             case TRIANGLE:
//                 return static_cast<short>(amplitude * (2.0 * fabs(2.0 * (cycles - floor(cycles + 0.5))) - 1.0));
//             default:
//                 return 0;
//         }
//     }
// }
