// preset_io.h
#ifndef PRESET_IO_H
#define PRESET_IO_H

#include <string>
#include <vector>

// 🎯 This is your core SynthPreset definition
struct SynthPreset {
    std::string name;

    float attack;
    float decay;
    float sustain;
    float release;
    float envAmount;

    float waveShape;

    float lfoRate;
    float lfoDepth;
    std::string lfoTarget;

    float chorusMix;
    float delayTime;
    float reverbAmount;
};

bool SavePresetToFile(const SynthPreset& preset, const std::string& path);
bool LoadPresetFromFile(SynthPreset& presetOut, const std::string& path);

#endif // PRESET_IO_H

