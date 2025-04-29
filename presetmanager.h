// presetmanager.h
#pragma once

#include <string>
#include <vector>

struct SynthPreset {
    uint32_t version = 1; // Always first field for future compatibility

    // Oscillator settings
    float oscSawLevel;
    float oscSquareLevel;
    float oscSineLevel;
    float oscSubLevel;
    float oscVolume;
    float oscPWMAmount;
    float oscMetalizerAmount;
    float oscUltrasawAmount;
    float oscSaturationAmount;
    float oscNoiseAmount;

    // ADSR settings
    float attackTime;
    float decayTime;
    float sustainLevel;
    float releaseTime;
    float envAmount;

    // LFO settings
    float lfoRate;
    float lfoDepth;
    int lfoWaveform; // 0 = Sine, 1 = Square, 2 = Triangle

    // Effects - Chorus
    bool chorusEnabled;
    float chorusRate;
    float chorusDepth;
    float chorusMix;

    // Effects - Delay
    bool delayEnabled;
    float delayTime;
    float delayFeedback;
    float delayMix;
    float delayHighCut;

    // Effects - Reverb
    bool reverbEnabled;
    float reverbDecay;
    float reverbDamping;
    float reverbMix;

    // Main waveform
    // int mainWaveform;
};

// Core preset functions
void SaveCurrentPreset(const std::string& filename);
bool LoadPreset(const std::string& filename);
std::vector<std::string> ListAvailablePresets(); // optional
