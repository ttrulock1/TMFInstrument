// preset_io.cpp
#include "preset_io.h"

#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool SavePresetToFile(const SynthPreset& preset, const std::string& path) {
    json j;
    j["name"] = preset.name;
    j["attack"] = preset.attack;
    j["decay"] = preset.decay;
    j["sustain"] = preset.sustain;
    j["release"] = preset.release;
    j["envAmount"] = preset.envAmount;

    j["waveShape"] = preset.waveShape;

    j["lfoRate"] = preset.lfoRate;
    j["lfoDepth"] = preset.lfoDepth;
    j["lfoTarget"] = preset.lfoTarget;

    j["chorusMix"] = preset.chorusMix;
    j["delayTime"] = preset.delayTime;
    j["reverbAmount"] = preset.reverbAmount;

    std::ofstream out(path);
    if (!out.is_open()) return false;

    out << j.dump(4); // pretty print with indentation
    return true;
}

bool LoadPresetFromFile(SynthPreset& presetOut, const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::stringstream buffer;
    buffer << in.rdbuf();
    json j = json::parse(buffer.str(), nullptr, false);
    if (j.is_discarded()) return false;

    presetOut.name = j.value("name", "");
    presetOut.attack = j.value("attack", 0.01f);
    presetOut.decay = j.value("decay", 0.1f);
    presetOut.sustain = j.value("sustain", 0.8f);
    presetOut.release = j.value("release", 0.1f);
    presetOut.envAmount = j.value("envAmount", 1.0f);

    presetOut.waveShape = j.value("waveShape", 0.0f);

    presetOut.lfoRate = j.value("lfoRate", 2.0f);
    presetOut.lfoDepth = j.value("lfoDepth", 0.5f);
    presetOut.lfoTarget = j.value("lfoTarget", "none");

    presetOut.chorusMix = j.value("chorusMix", 0.3f);
    presetOut.delayTime = j.value("delayTime", 0.2f);
    presetOut.reverbAmount = j.value("reverbAmount", 0.5f);

    return true;
}
