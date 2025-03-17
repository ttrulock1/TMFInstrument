#include <SDL2/SDL.h>
#include <thread>
#include <atomic>
#include "sound.h"
#include "shared_buffer.h"
#include <cstdint>  // For int16_t

// Global atomic flag to signal both threads to stop.
std::atomic<bool> quitFlag(false);

// Audio thread function.
void runAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Audio Init Failed: %s", SDL_GetError());
        return;
    }
    
    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 8192;
    
    // Audio callback using a lambda.
    spec.callback = +[](void* userdata, Uint8* stream, int len) -> void {
        int16_t* buffer = reinterpret_cast<int16_t*>(stream);
        int samples = len / sizeof(int16_t);
        static uint64_t totalSamples = 0;
        double frequency = 440.0;  // A4 note
        double amp = 0.5;
        sound::ADSR env = { 0.01, 0.1, 0.8, 0.1 };
        sound::WaveType waveType = sound::SINE;
        double duration = 1.0;
        
        for (int i = 0; i < samples; ++i) {
            double time = static_cast<double>(totalSamples++) / SAMPLE_RATE;
            short sample = sound::GenerateWave(waveType, time, duration, frequency, amp, env);
            buffer[i] = sample;
            // Push sample into the shared ring buffer.
            audioRingBuffer.push(sample);
        }
    };
    
    if (SDL_OpenAudio(&spec, nullptr) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    
    SDL_PauseAudio(0);
    
    // Run until quitFlag is true.
    while (!quitFlag.load()) {
        SDL_Delay(10);
    }
    
    SDL_CloseAudio();
    SDL_Quit();
}

// Visual thread function.
void runVisual() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("Video Init Failed: %s", SDL_GetError());
        return;
    }
    
    SDL_Window* window = SDL_CreateWindow("Oscilloscope",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 400, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }
    
    bool localQuit = false;
    while (!localQuit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                localQuit = true;
                quitFlag.store(true);
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int centerY = 200;  // Half of window height (400)
        int x = 0;
        int prevY = centerY;
        int16_t sample;  // Use int16_t to match the ring buffer type
        // Pop samples from the shared ring buffer and draw them.
        while (audioRingBuffer.pop(sample) && x < 800) {
            int y = centerY - (sample * centerY / 32767);
            SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            prevY = y;
            ++x;
        }
        
        SDL_RenderPresent(renderer);
        SDL_Delay(16);  // Roughly 60 FPS.
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main() {
    // Start audio and visual threads.
    std::thread audioThread(runAudio);
    std::thread visualThread(runVisual);
    
    // Wait for threads to finish.
    audioThread.join();
    visualThread.join();
    
    return 0;
}

