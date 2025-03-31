#include <SDL2/SDL.h>
#include "shared_buffer.h"
#include "sound.h"
#include <thread>

#include <cmath>

// External references from your existing files
extern std::atomic<sound::WaveType> currentWaveform;
extern RingBuffer<int16_t> audioRingBuffer;
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;

void playPadNote(int padIndex) {
    const double baseFreq = 261.63; // Middle C
    double freq = baseFreq * std::pow(2.0, padIndex / 12.0);
    double duration = 0.3;
    sound::ADSR env = {0.01, 0.05, 0.8, 0.05};

    NoteEvent note = {freq, duration, env};
    padNoteEvents.push(note); // Correct ✅
}



// void playPadNote(int padIndex) {
//     std::thread([padIndex]() {
//         const double baseFreq = 261.63; // Middle C
//         double freq = baseFreq * std::pow(2.0, padIndex / 12.0);
//         double duration = 0.3;
//         double amp = 0.5;
//         sound::ADSR env = {0.01, 0.05, 0.8, 0.05};

//         int samples = SAMPLE_RATE * duration;

//         for (int i = 0; i < samples; ++i) {
//             double time = static_cast<double>(i) / SAMPLE_RATE;
//             int16_t sample = sound::GenerateWave(
//                 currentWaveform.load(), time, duration, freq, amp, env
//             );

//             audioRingBuffer.push(sample);
//             SDL_Delay(1000 / SAMPLE_RATE);
//         }
//     }).detach();
// }

void DrawPads(SDL_Renderer* renderer) {
    const int padSize = 80;
    const int padding = 10;
    int startX = (WINDOW_WIDTH - (4 * padSize + 3 * padding)) / 2;
    int startY = (WINDOW_HEIGHT - (4 * padSize + 3 * padding)) / 2;

    for (int i = 0; i < 16; ++i) {
        int row = i / 4;
        int col = i % 4;

        SDL_Rect rect = {
            startX + col * (padSize + padding),
            startY + row * (padSize + padding),
            padSize,
            padSize
        };

        SDL_SetRenderDrawColor(renderer, 0, 100, 200, 255);
        SDL_RenderFillRect(renderer, &rect);
    }
}

void HandlePadEvents(SDL_Event& event) {
    const int padSize = 80;
    const int padding = 10;
    int startX = (WINDOW_WIDTH - (4 * padSize + 3 * padding)) / 2;
    int startY = (WINDOW_HEIGHT - (4 * padSize + 3 * padding)) / 2;

    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = event.button.x;
        int mouseY = event.button.y;

        for (int i = 0; i < 16; ++i) {
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
                playPadNote(i);  // Immediate playback
                break;
            }
        }
    }
}
