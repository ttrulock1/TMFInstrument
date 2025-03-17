#include <SDL2/SDL.h>
#include "shared_buffer.h"

void HandleStepToggle(SDL_Event event, bool steps[16]) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        int x = event.button.x;
        int stepIndex = x / (800 / 16);  // Determine which step was clicked
        steps[stepIndex] = !steps[stepIndex];  // Toggle step ON/OFF
    }
}

void DrawStepSequencer(SDL_Renderer* renderer, bool steps[16]) {
    int stepWidth = 800 / 16;
    int stepHeight = 50;
    int yPos = 350;  // Position near the bottom of the screen

    for (int i = 0; i < 16; i++) {
        SDL_Rect stepRect = { i * stepWidth, yPos, stepWidth - 5, stepHeight };
        if (steps[i]) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Active step (Green)
        } else {
            SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);  // Inactive step (Gray)
        }
        SDL_RenderFillRect(renderer, &stepRect);
    }
}

void StartOscilloscope(SDL_Renderer* renderer) {
    bool quit = false;

    while (!quit) {
        // Handle window events (close, ESC key)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }

            // ✅ Handle step toggling properly inside event loop
            HandleStepToggle(event, stepSequence);
        }

        // Clear screen 
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Draw waveform in green
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        int centerY = h / 2;
        int x = 0;
        int prevY = centerY;
        int16_t sample;

        // Pull samples from the audio ring buffer and draw the waveform
        while (audioRingBuffer.pop(sample) && x < w) {
            int y = centerY - (sample * centerY / 32767);
            if (x > 0) {
                SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            }
            prevY = y;
            ++x;
        }

        // ✅ Call Step Sequencer Drawing
        DrawStepSequencer(renderer, stepSequence);

        SDL_RenderPresent(renderer);  // Present frame (vsync throttles to ~60 FPS)
    }
}
