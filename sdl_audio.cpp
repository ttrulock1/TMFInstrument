#include <SDL2/SDL.h>
#include <iostream>
#include "sound.h"
#include "shared_buffer.h"

// Buffer size for SDL audio callback.
const int BUFFER_SIZE = 8192;
static uint64_t totalSamples = 0;  // Continuous sample counter

void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    // Synthesis parameters
    double frequency = 440.0;  // A4 note
    double amp = 0.5;
    sound::ADSR env = { 0.01, 0.1, 0.8, 0.1 };
    sound::WaveType waveType = sound::SINE;
    double duration = 1.0;

    std::cout << "Audio Callback Debug: First 10 Samples = ";
    for (int i = 0; i < samples; ++i) {
        double time = static_cast<double>(totalSamples++) / SAMPLE_RATE;
        short sample = sound::GenerateWave(waveType, time, duration, frequency, amp, env);
        buffer[i] = sample;
        audioRingBuffer.push(sample);

        // Print the first 10 samples
        if (i < 10) {
            std::cout << sample << " ";
        }
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Failed to initialize SDL audio: %s", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = BUFFER_SIZE;
    spec.callback = AudioCallback;

    if (SDL_OpenAudio(&spec, nullptr) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_PauseAudio(0);
    SDL_Delay(5000);  // Run for 5 seconds.
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}
