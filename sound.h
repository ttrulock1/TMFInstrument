#include <math.h>

namespace sound {
    #define TWOPI 6.28

    short SineWave(double time, double frequency, double amp){
        short result;
        double tpc = 44100/frequency; //ticks per cycle
        double rad = TWOPI * cycles;
        short amplitude = 32767 * amp;
        result = amplitude * sin(rad);
        return result; 

    }
}