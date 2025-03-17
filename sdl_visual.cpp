#include <SDL2/SDL.h>
#include <iostream>
#include "shared_buffer.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 400;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Oscilloscope",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
               (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // ✅ This version reads from `audioRingBuffer`, but the buffer had no data
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int centerY = WINDOW_HEIGHT / 2;
        int x = 0;
        int prevY = centerY;
        int16_t sample;

        std::cout << "Oscilloscope Debug: First 10 Samples = ";
        int debugCount = 0;

        while (audioRingBuffer.pop(sample) && x < WINDOW_WIDTH) {
            // Print the first 10 samples for debugging
            if (debugCount < 10) {
                std::cout << sample << " ";
                debugCount++;
            }

            int y = centerY - (sample * centerY / 32767);
            if (x > 0) {
                SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            }
            prevY = y;
            ++x;
        }

        std::cout << std::endl;

        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // ~60 FPS throttle
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
