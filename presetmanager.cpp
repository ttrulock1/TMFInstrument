// presetmanager.cpp
#include "presetmanager.h"
#include "shared_buffer.h"
#include "lfo_engine.h"
#include "oscillator_ui.h"  
#include <fstream>
#include <filesystem> // for optional preset listing
namespace fs = std::filesystem;

// Save current synth settings into a preset file
void SaveCurrentPreset(const std::string& filename) {
    SynthPreset preset;

    // Fill from shared_buffer.h
    preset.oscSawLevel = oscSawLevel.load();
    preset.oscSquareLevel = oscSquareLevel.load();
    preset.oscSineLevel = oscSineLevel.load();
    preset.oscSubLevel = oscSubLevel.load();
    preset.oscVolume = oscVolume.load();
    preset.oscPWMAmount = oscPWMAmount.load();
    preset.oscMetalizerAmount = oscMetalizerAmount.load();
    preset.oscUltrasawAmount = oscUltrasawAmount.load();
    preset.oscSaturationAmount = oscSaturationAmount.load();
    preset.oscNoiseAmount = oscNoiseAmount.load();

    preset.attackTime = uiAttackTime.load();
    preset.decayTime = uiDecayTime.load();
    preset.sustainLevel = uiSustainLevel.load();
    preset.releaseTime = uiReleaseTime.load();
    preset.envAmount = uiEnvAmount.load();

    preset.lfoRate = lfo.rateHz;
    preset.lfoDepth = lfo.depth;
    preset.lfoWaveform = static_cast<int>(lfo.waveform);

    preset.chorusEnabled = chorusEnabled.load();
    preset.chorusRate = chorusRate.load();
    preset.chorusDepth = chorusDepth.load();
    preset.chorusMix = chorusMix.load();

    preset.delayEnabled = delayEnabled.load();
    preset.delayTime = delayTime.load();
    preset.delayFeedback = delayFeedback.load();
    preset.delayMix = delayMix.load();
    preset.delayAnalogMode = delayAnalogMode.load();


    preset.reverbEnabled = reverbEnabled.load();
    preset.reverbDecay = reverbDecay.load();
    preset.reverbDamping = reverbDamping.load();
    preset.reverbMix = reverbMix.load();

    // preset.mainWaveform = currentWaveform.load();

    // Save binary
    std::ofstream out(filename, std::ios::binary);
    if (out.is_open()) {
        out.write(reinterpret_cast<const char*>(&preset), sizeof(SynthPreset));
    }
}

// Load a preset file into synth settings
bool LoadPreset(const std::string& filename) {
    SynthPreset preset;
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    in.read(reinterpret_cast<char*>(&preset), sizeof(SynthPreset));
    if (preset.version > 1) {
        return false; // unsupported version (future-proofing)
    }

    // Apply to shared buffer
    oscSawLevel.store(preset.oscSawLevel);
    oscSquareLevel.store(preset.oscSquareLevel);
    oscSineLevel.store(preset.oscSineLevel);
    oscSubLevel.store(preset.oscSubLevel);
    oscVolume.store(preset.oscVolume);
    oscPWMAmount.store(preset.oscPWMAmount);
    oscMetalizerAmount.store(preset.oscMetalizerAmount);
    oscUltrasawAmount.store(preset.oscUltrasawAmount);
    oscSaturationAmount.store(preset.oscSaturationAmount);
    oscNoiseAmount.store(preset.oscNoiseAmount);

    uiAttackTime.store(preset.attackTime);
    uiDecayTime.store(preset.decayTime);
    uiSustainLevel.store(preset.sustainLevel);
    uiReleaseTime.store(preset.releaseTime);
    uiEnvAmount.store(preset.envAmount);

    // 👻 ghost: load LFO
    lfo.rateHz = preset.lfoRate;
    lfo.depth = preset.lfoDepth;
    lfo.waveform = static_cast<LFOWaveform>(preset.lfoWaveform);



    chorusEnabled.store(preset.chorusEnabled);
    chorusRate.store(preset.chorusRate);
    chorusDepth.store(preset.chorusDepth);
    chorusMix.store(preset.chorusMix);

    delayEnabled.store(preset.delayEnabled);
    delayTime.store(preset.delayTime);
    delayFeedback.store(preset.delayFeedback);
    delayMix.store(preset.delayMix);
    delayAnalogMode.store(preset.delayAnalogMode);

    reverbEnabled.store(preset.reverbEnabled);
    reverbDecay.store(preset.reverbDecay);
    reverbDamping.store(preset.reverbDamping);
    reverbMix.store(preset.reverbMix);

    // currentWaveform.store(preset.mainWaveform);
    return true;
}

// (Optional) list available presets
std::vector<std::string> ListAvailablePresets() {
    std::vector<std::string> presets;
    if (!fs::exists("presets/"))
        return presets;

    for (auto& entry : fs::directory_iterator("presets/")) {
        if (entry.is_regular_file()) {
            presets.push_back(entry.path().string());
        }
    }
    return presets;
}
