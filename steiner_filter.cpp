#include "steiner_filter.h"
#include <cmath>
#include <algorithm> // for std::clamp (safely limits values)

SteinerFilter::SteinerFilter(double sampleRate)
    : sampleRate(sampleRate) {} // sets sample rate when creating filter


void SteinerFilter::setCutoff(float hz) {
    cutoffHz = hz;
}

float SteinerFilter::process(float input, float resonance, float vca) {
    // Limit cutoff range to safe values
    float maxCutoff = static_cast<float>(sampleRate * 0.5);
    float cutoff = std::clamp(cutoffHz, 20.0f, maxCutoff);

    float f = std::clamp(static_cast<float>((2.0f * cutoff) / sampleRate), 0.0f, 1.0f);

    // Calculate feedback amount from resonance (adds peak emphasis)

    float fb = std::clamp(resonance * 4.0f * (1.0f - 0.15f * f * f), 0.0f, 4.0f);
    input -= buf3 * fb;

    buf0 += f * (input - buf0);
    buf1 += f * (buf0 - buf1);
    buf2 += f * (buf1 - buf2);
    buf3 += f * (buf2 - buf3);

        // Apply VCA (volume control after filtering)
        //SHOULD THIS BE AFTER OR BEFORE FILTER? Also shoul this be here of the FFD.cpp?r
    float output = buf3 * vca;
    output *= 1.5f;
    // return std::clamp(output, -1.0f, 1.0f);
    return output; // allow hot feedback

}
