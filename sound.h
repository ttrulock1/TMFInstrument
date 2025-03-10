#include <math.h>

namespace sound {
    #define TWOPI 6.28318530718  // More precise Pi * 2

    short SineWave(double time, double frequency, double amp){
        double tpc = 44100/frequency; //ticks per cycle
        double cycles = time / tpc;
        double rad = TWOPI * cycles;
        short amplitude = static_cast<short>(32767 * amp);
        return static_cast<short>(amplitude * std::sin(rad)); 
    }
}

#include <cmath>
