#include "save_switch.h"
#include "shared_buffer.h"
#include "sound.h"

#include <cstring>
#include <sstream>
#include <iostream>

#ifdef __NX__  // Official Nintendo SDK path
#include <nn/fs.h>
#include <nn/fs/fs_MountSaveData.h>
#include <nn/err.h>
#include <nn/os.h>

namespace {
    const char* kSaveMountName = "SaveData";
    const char* kSaveFilePath = "/SaveData/save.dat";
}

bool MountSaveDataIfNeeded() {
    static bool mounted = false;
    if (mounted) return true;

    nn::Result result = nn::fs::MountSaveData(kSaveMountName);
    if (result.IsSuccess()) {
        mounted = true;
        return true;
    } else {
        std::cerr << "MountSaveData failed: 0x" << std::hex << result.GetValue() << std::endl;
        return false;
    }
}

bool SaveSynthState() {
    if (!MountSaveDataIfNeeded()) return false;

    nn::fs::FileHandle handle;
    nn::fs::DirectoryEntryType entryType;
    nn::fs::Result result;

    // Open (or create) the file
    result = nn::fs::OpenFile(&handle, kSaveFilePath,
        nn::fs::OpenMode_Read | nn::fs::OpenMode_Write);
    if (!result.IsSuccess()) {
        // Try to create file if it doesn't exist
        result = nn::fs::CreateFile(kSaveFilePath, 2048);
        if (!result.IsSuccess()) return false;

        result = nn::fs::OpenFile(&handle, kSaveFilePath,
            nn::fs::OpenMode_Read | nn::fs::OpenMode_Write);
        if (!result.IsSuccess()) return false;
    }

    std::ostringstream oss;
    oss << BPM.load() << "\n";
    oss << static_cast<int>(currentWaveform.load()) << "\n";

    for (int i = 0; i < 16; ++i) oss << (stepSequence[i] ? 1 : 0) << " ";
    oss << "\n";

    for (int i = 0; i < 16; ++i) oss << stepPitches[i].load() << " ";
    oss << "\n";

    oss << morphPosition.load() << "\n";
    oss << overtone.load() << "\n";
    oss << metalizer.load() << "\n";
    oss << bruteFactor.load() << "\n";
    oss << waveTilt.load() << "\n";
    oss << pulseWidth.load() << "\n";
    oss << symmetry.load() << "\n";

    std::string data = oss.str();
    size_t size = data.size();

    result = nn::fs::WriteFile(handle, 0, data.c_str(), size,
                               nn::fs::WriteOption::Flush);
    nn::fs::CloseFile(handle);
    return result.IsSuccess();
}

bool LoadSynthState() {
    if (!MountSaveDataIfNeeded()) return false;

    nn::fs::FileHandle handle;
    nn::fs::Result result = nn::fs::OpenFile(&handle, kSaveFilePath, nn::fs::OpenMode_Read);
    if (!result.IsSuccess()) {
        std::cerr << "No save file found on Switch.\n";
        return false;
    }

    char buffer[2048] = {0};
    int64_t bytesRead = 0;
    result = nn::fs::ReadFile(&bytesRead, handle, 0, buffer, sizeof(buffer));
    nn::fs::CloseFile(handle);
    if (!result.IsSuccess()) return false;

    std::istringstream iss(std::string(buffer, bytesRead));
    std::string line;

    if (!std::getline(iss, line)) return false;
    BPM.store(std::stoi(line));

    if (!std::getline(iss, line)) return false;
    currentWaveform.store(static_cast<sound::WaveType>(std::stoi(line)));

    if (!std::getline(iss, line)) return false;
    {
        std::istringstream s(line);
        for (int i = 0; i < 16; ++i) {
            int v;
            s >> v;
            stepSequence[i] = (v != 0);
        }
    }

    if (!std::getline(iss, line)) return false;
    {
        std::istringstream s(line);
        for (int i = 0; i < 16; ++i) {
            int v;
            s >> v;
            stepPitches[i].store(v);
        }
    }

    if (!std::getline(iss, line)) return false;
    morphPosition.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    overtone.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    metalizer.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    bruteFactor.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    waveTilt.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    pulseWidth.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    symmetry.store(std::stof(line));

    return true;
}

#else // PC fallback for local testing

#include "SDL.h"

bool SaveSynthState() {
    SDL_RWops* file = SDL_RWFromFile("save.dat", "w");
    if (!file) return false;

    std::ostringstream oss;
    oss << BPM.load() << "\n";
    oss << static_cast<int>(currentWaveform.load()) << "\n";

    for (int i = 0; i < 16; ++i) oss << (stepSequence[i] ? 1 : 0) << " ";
    oss << "\n";

    for (int i = 0; i < 16; ++i) oss << stepPitches[i].load() << " ";
    oss << "\n";

    oss << morphPosition.load() << "\n";
    oss << overtone.load() << "\n";
    oss << metalizer.load() << "\n";
    oss << bruteFactor.load() << "\n";
    oss << waveTilt.load() << "\n";
    oss << pulseWidth.load() << "\n";
    oss << symmetry.load() << "\n";

    std::string out = oss.str();
    SDL_RWwrite(file, out.c_str(), 1, out.size());
    SDL_RWclose(file);
    return true;
}

bool LoadSynthState() {
    SDL_RWops* file = SDL_RWFromFile("save.dat", "r");
    if (!file) return false;

    Sint64 size = SDL_RWsize(file);
    char* buffer = new char[size + 1];
    SDL_RWread(file, buffer, 1, size);
    buffer[size] = '\0';
    SDL_RWclose(file);

    std::istringstream iss(buffer);
    std::string line;

    if (!std::getline(iss, line)) return false;
    BPM.store(std::stoi(line));

    if (!std::getline(iss, line)) return false;
    currentWaveform.store(static_cast<sound::WaveType>(std::stoi(line)));

    if (!std::getline(iss, line)) return false;
    {
        std::istringstream s(line);
        for (int i = 0; i < 16; ++i) {
            int v;
            s >> v;
            stepSequence[i] = (v != 0);
        }
    }

    if (!std::getline(iss, line)) return false;
    {
        std::istringstream s(line);
        for (int i = 0; i < 16; ++i) {
            int v;
            s >> v;
            stepPitches[i].store(v);
        }
    }

    if (!std::getline(iss, line)) return false;
    morphPosition.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    overtone.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    metalizer.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    bruteFactor.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    waveTilt.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    pulseWidth.store(std::stof(line));
    if (!std::getline(iss, line)) return false;
    symmetry.store(std::stof(line));

    delete[] buffer;
    return true;
}
#endif
