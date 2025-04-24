#include <SDL2/SDL.h>
#include "shared_buffer.h"
#include "sound.h"
#include "arp.h"
#include <thread>
#include <cmath>
#include <atomic> // ✅ NEW: Required for std::atomic<bool>

// External references
extern std::atomic<sound::WaveType> currentWaveform;
extern RingBuffer<int16_t> audioRingBuffer;

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;
const int NUM_PADS = 16;

// ✅ NEW: Track which pads are held down
std::atomic<bool> padHeld[NUM_PADS];

// ✅ NEW: Track how long each pad note has been held
double padStartTime[NUM_PADS]; // in seconds

// ✅ NEW: Called when a pad is pressed — creates the note but defers release
void triggerPadNoteOn(int padIndex) {
    const double baseFreq = 261.63; // Middle C
    double freq = baseFreq * std::pow(2.0, padIndex / 12.0);
    ADSR env = {0.01f, 0.05f, 0.8f, 0.3f}; // 🌹 fixed to match adsr_engine.h

    // Save note-on time (for later calculation of duration)
    padStartTime[padIndex] = SDL_GetTicks() / 1000.0; // Convert to seconds
    padHeld[padIndex] = true;

    // Push initial note event with dummy duration (will be updated on release)
    NoteEvent note = {freq, 999.0, env};  // 🔥 duration will be overridden in audio thread
    padNoteEvents.push(note);
}

// ✅ NEW: Called when a pad is released — triggers duration stop
void triggerPadNoteOff(int padIndex) {
    if (padHeld[padIndex]) {
        padHeld[padIndex] = false;

        // Calculate real duration
        double endTime = SDL_GetTicks() / 1000.0;
        double duration = endTime - padStartTime[padIndex];

        // 🔥 Push updated duration via release signal
        NoteEvent releaseNote;
        releaseNote.frequency = -padIndex;  // Negative freq = signal to update/release note
        releaseNote.duration = duration;
        padNoteEvents.push(releaseNote);  // Audio thread must handle this properly
    }
}

// ✅ OLD: Replaced by triggerPadNoteOn + triggerPadNoteOff
/*
void playPadNote(int padIndex) {
    const double baseFreq = 261.63; // Middle C
    double freq = baseFreq * std::pow(2.0, padIndex / 12.0);
    double duration = 0.3;
    sound::ADSR env = {0.01, 0.05, 0.8, 0.05};

    NoteEvent note = {freq, duration, env};
    padNoteEvents.push(note); // Correct ✅
}
*/

// 🎨 Draw the grid of pads
void DrawPads(SDL_Renderer* renderer) {
    const int padSize = 80;
    const int padding = 10;
    int startX = (WINDOW_WIDTH - (4 * padSize + 3 * padding)) / 2;
    int startY = (WINDOW_HEIGHT - (4 * padSize + 3 * padding)) / 2;

    for (int i = 0; i < NUM_PADS; ++i) {
        int row = i / 4;
        int col = i % 4;

        SDL_Rect rect = {
            startX + col * (padSize + padding),
            startY + row * (padSize + padding),
            padSize,
            padSize
        };

        // 🔵 Light up if held
        if (padHeld[i]) {
            SDL_SetRenderDrawColor(renderer, 0, 200, 100, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
        }

        SDL_RenderFillRect(renderer, &rect);
    }
}

// 🖱 Handle pad clicks/releases
void HandlePadEvents(SDL_Event& event) {
    const int padSize = 80;
    const int padding = 10;
    int startX = (WINDOW_WIDTH - (4 * padSize + 3 * padding)) / 2;
    int startY = (WINDOW_HEIGHT - (4 * padSize + 3 * padding)) / 2;

    int mouseX = event.button.x;
    int mouseY = event.button.y;

    for (int i = 0; i < NUM_PADS; ++i) {
        int row = i / 4;
        int col = i % 4;

        SDL_Rect rect = {
            startX + col * (padSize + padding),
            startY + row * (padSize + padding),
            padSize,
            padSize
        };

        SDL_Point clickPoint = {mouseX, mouseY};

        if (SDL_PointInRect(&clickPoint, &rect)) {
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                triggerPadNoteOn(i); // ✅ Trigger note ON
            } else if (event.type == SDL_MOUSEBUTTONUP) {
                triggerPadNoteOff(i); // ✅ Trigger note OFF
            }
            break;
        }
    }
}
