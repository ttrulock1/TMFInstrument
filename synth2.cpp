#include <cmath>
#include "sound.h"

void GenerateSineWave(int16_t* pData, int nNumSamples, double frequency, double amp) {
    for (int nIndex = 0; nIndex < nNumSamples; ++nIndex) {
        double time = static_cast<double>(nIndex) / 44100.0;  // Convert index to time
        pData[nIndex] = sound::SineWave(time, frequency, amp);
    }
}