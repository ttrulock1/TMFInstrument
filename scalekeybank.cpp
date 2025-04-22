#include "scalekeybank.h"
#include <cmath>

// Constructor: Initialize with default scales and set initial scale/key
ScaleBank::ScaleBank()
    : selectedScaleIndex(0), selectedKeyIndex(0) {
    // Common scales
    scales.push_back({"Major", {0, 2, 4, 5, 7, 9, 11}});
    scales.push_back({"Minor", {0, 2, 3, 5, 7, 8, 10}});
    scales.push_back({"Pentatonic", {0, 2, 4, 7, 9}});
    scales.push_back({"Blues", {0, 3, 5, 6, 7, 10}});
    scales.push_back({"Chromatic", {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}});
}

void ScaleBank::addScale(const std::string& name, const std::vector<int>& intervals) {
    scales.push_back({name, intervals});
}

void ScaleBank::setSelectedScale(int index) {
    if (index >= 0 && index < static_cast<int>(scales.size())) {
        selectedScaleIndex = index;
    }
}

void ScaleBank::setSelectedKey(int index) {
    if (index >= 0 && index < 12) {
        selectedKeyIndex = index;
    }
}

double ScaleBank::applyScale(double frequency) const {
    const Scale& scale = scales[selectedScaleIndex];

    // Convert frequency to MIDI note
    double midiFloat = 69.0 + 12.0 * std::log2(frequency / 440.0);
    int midiNote = static_cast<int>(std::round(midiFloat));

    // Define the root MIDI note (middle C = 60) shifted by selected key
    int rootMidi = 60 + selectedKeyIndex;

    // Expand the scale through ~4 octaves (enough to cover 25 notes)
    std::vector<int> candidateNotes;
    for (int octave = -2; octave <= 2; ++octave) {
        int base = rootMidi + octave * 12;
        for (int interval : scale.intervals) {
            int note = base + interval;
            if (note >= 0 && note <= 127)
                candidateNotes.push_back(note);
        }
    }

    // Find the closest match
    int bestNote = midiNote;
    int smallestDiff = 128;

    for (int candidate : candidateNotes) {
        int diff = std::abs(candidate - midiNote);
        if (diff < smallestDiff) {
            bestNote = candidate;
            smallestDiff = diff;
        }
    }

    // Convert back to frequency
    return 440.0 * std::pow(2.0, (bestNote - 69) / 12.0);
}


void ScaleBank::nextScale() {
    selectedScaleIndex = (selectedScaleIndex + 1) % scales.size();
}

void ScaleBank::nextKey() {
    selectedKeyIndex = (selectedKeyIndex + 1) % 12;
}

int ScaleBank::getSelectedScaleIndex() const {
    return selectedScaleIndex;
}

int ScaleBank::getSelectedKeyIndex() const {
    return selectedKeyIndex;
}

const Scale& ScaleBank::getSelectedScale() const {
    return scales[selectedScaleIndex];
}

std::string ScaleBank::getSelectedKeyName() const {
    static const std::vector<std::string> KEYS = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    return KEYS[selectedKeyIndex % KEYS.size()];
}

int ScaleBank::getScaleCount() const {
    return static_cast<int>(scales.size());
}

std::string ScaleBank::getScaleName(int index) const {
    if (index >= 0 && index < static_cast<int>(scales.size())) {
        return scales[index].name;
    }
    return "Invalid";
}
