#ifndef ADSR_BACKEND_H
#define ADSR_BACKEND_H

#include <string>
#include <vector>

struct ADSRParams {
    float attackTime  = 0.01f;
    float decayTime   = 0.1f;
    float sustainLevel = 0.8f;
    float releaseTime = 0.2f;
    float amount      = 1.0f;
};

// Preset container
struct ADSRPreset {
    std::string name;
    ADSRParams params;
};

// Core API
namespace ADSRBackend {
    void LoadDefault(); // Load built-in default values

    void SetParams(const ADSRParams& newParams);
    ADSRParams GetParams(); // Fetch current backend state

    void SavePreset(const std::string& name);
    bool LoadPreset(const std::string& name);

    const std::vector<ADSRPreset>& GetAllPresets(); // For UI listing
}

#endif // ADSR_BACKEND_H
