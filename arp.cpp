// arp.cpp
#include "arp.h"
#include "shared_buffer.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <chrono>
#include "adsr_engine.h" // for NoteEvent

namespace {
    std::vector<double> notesHeld;
    int arpIndex = 0;
    int stepSamples = 0;
    int currentSample = 0;
    double sampleRate = 44100.0;
    bool arpOn = false;
    int bpm = 120;
    int divisions = 4; // 16th notes

    enum ArpMode { Up, Down, Random, AsPlayed };
    ArpMode currentMode = Up;
    std::vector<double> playOrder;
    std::default_random_engine rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
}

void Arp_Init(double rate) {
    sampleRate = rate;
    notesHeld.clear();
    playOrder.clear();
    arpIndex = 0;
    currentSample = 0;
    Arp_SetBPM(bpm);
    arpOn = false;
}

void Arp_SetBPM(int newBPM) {
    bpm = newBPM;
    stepSamples = static_cast<int>((60.0 / bpm) * sampleRate / divisions);
}

void Arp_SetRate(int divs) {
    divisions = divs;
    Arp_SetBPM(bpm); // recalc stepSamples
}

void Arp_SetMode(int mode) {
    currentMode = static_cast<ArpMode>(mode);
}

void Arp_AddNote(double freq) {
    if (std::find(notesHeld.begin(), notesHeld.end(), freq) == notesHeld.end()) {
        notesHeld.push_back(freq);
        playOrder.push_back(freq);

        if (currentMode == Up) std::sort(notesHeld.begin(), notesHeld.end());
        if (currentMode == Down) std::sort(notesHeld.begin(), notesHeld.end(), std::greater<double>());

        arpOn = true;
    }
}

void Arp_RemoveNote(double freq) {
    auto it = std::remove(notesHeld.begin(), notesHeld.end(), freq);
    notesHeld.erase(it, notesHeld.end());

    auto po = std::remove(playOrder.begin(), playOrder.end(), freq);
    playOrder.erase(po, playOrder.end());

    if (notesHeld.empty()) {
        arpOn = false;
    }
}

bool Arp_IsActive() {
    return arpOn;
}

void Arp_Enable(bool on) {
    arpOn = on;
}

void Arp_Update() {
    if (!arpOn || notesHeld.empty()) return;

    currentSample++;
    if (currentSample >= stepSamples) {
        currentSample = 0;
        double freq = 0.0;

        switch (currentMode) {
            case Up:
            case Down:
                freq = notesHeld[arpIndex % notesHeld.size()];
                break;
            case Random:
                freq = notesHeld[rng() % notesHeld.size()];
                break;
            case AsPlayed:
                freq = playOrder[arpIndex % playOrder.size()];
                break;
        }

        arpIndex++;

        ADSR env = {0.01f, 0.05f, 0.8f, 0.3f};
        NoteEvent note = {freq, 0.2, env};
        padNoteEvents.push(note);
    }
}
