#include <SDL2/SDL.h>
#include <cmath>
#include "sound.h"

const int SAMPLE_RATE = 44100;
const int BUFFER_SIZE = 4096;

void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);
    double frequency = 440.0;  // A4 note
    double amp = 0.5;

    for (int i = 0; i < samples; ++i) {
        double time = static_cast<double>(i) / SAMPLE_RATE;
        buffer[i] = sound::SineWave(time, frequency, amp);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
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
    SDL_Delay(5000);  // Play for 5 seconds
    SDL_CloseAudio();
    SDL_Quit();

    return 0;
}
