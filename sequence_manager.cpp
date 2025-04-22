#include "sequencemanager.h"
#include <fstream>
#include <sstream>
#include <iostream>

SequenceManager::SequenceManager() : activeSequenceIndex(0) {}

Sequence& SequenceManager::getSequence(int index) {
    return sequences.at(index); // bounds-checked
}

void SequenceManager::setActiveSequence(int index) {
    if (index >= 0 && index < sequences.size()) {
        activeSequenceIndex = index;
    }
}

int SequenceManager::getActiveSequenceIndex() const {
    return activeSequenceIndex;
}

bool SequenceManager::SaveToFile(const std::string& filename) {
    std::ofstream out(filename);
    if (!out.is_open()) return false;

    out << activeSequenceIndex << "\n";

    for (const auto& seq : sequences) {
        out << seq.name << "\n";
        for (int i = 0; i < Sequence::MAX_STEPS; ++i) out << seq.steps[i] << " ";
        out << "\n";
        for (int i = 0; i < Sequence::MAX_STEPS; ++i) out << seq.pitchOffset[i] << " ";
        out << "\n";
        out << seq.scaleIndex << " " << seq.keyIndex << "\n";
    }
    return true;
}

bool SequenceManager::LoadFromFile(const std::string& filename) {
    std::ifstream in(filename);
    if (!in.is_open()) return false;

    std::string line;
    if (!std::getline(in, line)) return false;
    activeSequenceIndex = std::stoi(line);

    for (auto& seq : sequences) {
        if (!std::getline(in, seq.name)) return false;

        if (!std::getline(in, line)) return false;
        std::istringstream stepLine(line);
        for (int i = 0; i < Sequence::MAX_STEPS; ++i) stepLine >> seq.steps[i];

        if (!std::getline(in, line)) return false;
        std::istringstream pitchLine(line);
        for (int i = 0; i < Sequence::MAX_STEPS; ++i) pitchLine >> seq.pitchOffset[i];

        if (!std::getline(in, line)) return false;
        std::istringstream scaleLine(line);
        scaleLine >> seq.scaleIndex >> seq.keyIndex;
    }
    return true;
}
