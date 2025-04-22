#ifdef __NX__

#include "sequencemanager.h"

#include <nn/fs.h>
#include <nn/fs/fs_MountSaveData.h>
#include <nn/err.h>
#include <nn/os.h>
#include <sstream>
#include <iostream>

namespace {
    const char* kSeqPath = "/SaveData/sequences.txt";
    const size_t kBufferSize = 4096;
    char readBuffer[kBufferSize];

    std::string SerializeSequences(const SequenceManager& manager) {
        std::ostringstream oss;
        oss << manager.getActiveSequenceIndex() << "\n";

        for (int i = 0; i < 48; ++i) {
            const auto& seq = manager.getSequence(i);
            oss << seq.name << "\n";
            for (int j = 0; j < Sequence::MAX_STEPS; ++j) oss << seq.steps[j] << " ";
            oss << "\n";
            for (int j = 0; j < Sequence::MAX_STEPS; ++j) oss << seq.pitchOffset[j] << " ";
            oss << "\n";
            oss << seq.scaleIndex << " " << seq.keyIndex << "\n";
        }

        return oss.str();
    }

    bool DeserializeSequences(SequenceManager& manager, const std::string& data) {
        std::istringstream iss(data);
        std::string line;

        if (!std::getline(iss, line)) return false;
        manager.setActiveSequence(std::stoi(line));

        for (int i = 0; i < 48; ++i) {
            auto& seq = manager.getSequence(i);
            if (!std::getline(iss, seq.name)) return false;

            if (!std::getline(iss, line)) return false;
            std::istringstream steps(line);
            for (int j = 0; j < Sequence::MAX_STEPS; ++j) steps >> seq.steps[j];

            if (!std::getline(iss, line)) return false;
            std::istringstream pitches(line);
            for (int j = 0; j < Sequence::MAX_STEPS; ++j) pitches >> seq.pitchOffset[j];

            if (!std::getline(iss, line)) return false;
            std::istringstream scale(line);
            scale >> seq.scaleIndex >> seq.keyIndex;
        }

        return true;
    }
}

// SWITCH platform-specific file I/O
bool SequenceManager::SaveToFile(const std::string& unused) {
    nn::fs::FileHandle handle;
    nn::Result result;

    result = nn::fs::CreateFile(kSeqPath, kBufferSize);
    if (!result.IsSuccess()) {
        result = nn::fs::OpenFile(&handle, kSeqPath, nn::fs::OpenMode_Read | nn::fs::OpenMode_Write);
        if (!result.IsSuccess()) {
            std::cerr << "Failed to open sequence file: 0x" << std::hex << result.GetValue() << std::endl;
            return false;
        }
    } else {
        result = nn::fs::OpenFile(&handle, kSeqPath, nn::fs::OpenMode_Read | nn::fs::OpenMode_Write);
    }

    std::string data = SerializeSequences(*this);
    result = nn::fs::WriteFile(handle, 0, data.c_str(), data.size(), nn::fs::WriteOption::Flush);
    nn::fs::CloseFile(handle);
    return result.IsSuccess();
}

bool SequenceManager::LoadFromFile(const std::string& unused) {
    nn::fs::FileHandle handle;
    nn::Result result = nn::fs::OpenFile(&handle, kSeqPath, nn::fs::OpenMode_Read);
    if (!result.IsSuccess()) {
        std::cerr << "Sequence file not found on Switch.\n";
        return false;
    }

    int64_t bytesRead = 0;
    result = nn::fs::ReadFile(&bytesRead, handle, 0, readBuffer, kBufferSize);
    nn::fs::CloseFile(handle);
    if (!result.IsSuccess() || bytesRead == 0) return false;

    std::string data(readBuffer, bytesRead);
    return DeserializeSequences(*this, data);
}

#endif // __NX__
