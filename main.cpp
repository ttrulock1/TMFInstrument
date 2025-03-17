#include <SDL2/SDL.h>
#include <iostream>
#include <atomic>
#include "sound.h"
#include "shared_buffer.h"

// Global quit flag to signal threads to stop (used if needed)
std::atomic<bool> quitFlag(false);

// Forward declare audio control functions
bool loadWavFile(const char* filename, SDL_AudioSpec& spec, Uint8*& buffer, Uint32& length);
bool startAudio(SDL_AudioSpec& desiredSpec, Uint8* wavBuffer, Uint32 wavLength);
void closeAudio();

// Static shared data for audio callback
static Uint8* audioData = nullptr;
static Uint32 audioDataLen = 0;
static Uint32 frameIndex = 0;
static Uint32 totalFrames = 0;
static int audioChannels = 1;
static uint64_t totalSamplesGenerated = 0;

// Audio callback function (called on SDL’s audio thread)
static void SDLAudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* out = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    if (audioData != nullptr) {
        // Playback from loaded WAV data
        int framesToWrite = (audioChannels > 0) ? samples / audioChannels : samples;
        for (int i = 0; i < framesToWrite; ++i) {
            if (frameIndex < totalFrames) {
                if (audioChannels == 1) {
                    // Mono: copy one sample
                    int16_t sample = reinterpret_cast<int16_t*>(audioData)[frameIndex];
                    out[i] = sample;
                    audioRingBuffer.push(sample);
                } else if (audioChannels == 2) {
                    // Stereo: copy left and right
                    int base = frameIndex * 2;
                    int16_t left = reinterpret_cast<int16_t*>(audioData)[base];
                    int16_t right = reinterpret_cast<int16_t*>(audioData)[base + 1];
                    out[2 * i] = left;
                    out[2 * i + 1] = right;
                    audioRingBuffer.push(left);  // push left channel for visualization
                }
                frameIndex++;
            } else {
                // End of audio data reached – output silence
                if (audioChannels == 1) {
                    out[i] = 0;
                } else if (audioChannels == 2) {
                    out[2 * i] = 0;
                    out[2 * i + 1] = 0;
                }
                audioRingBuffer.push(0);
            }
        }
    } else {
        // Synthesize audio (440 Hz sine wave with ADSR envelope)
        double frequency = 440.0;
        double amp = 0.5;
        sound::ADSR env = {0.01, 0.1, 0.8, 0.1};  // Attack, Decay, Sustain, Release
        sound::WaveType waveType = sound::SINE;
        double duration = 1.0;  // 1-second note duration

        for (int i = 0; i < samples; ++i) {
            double time = static_cast<double>(totalSamplesGenerated++) / SAMPLE_RATE;
            int16_t sample = sound::GenerateWave(waveType, time, duration, frequency, amp, env);
            out[i] = sample;
            audioRingBuffer.push(sample);
        }
    }
}

bool loadWavFile(const char* filename, SDL_AudioSpec& spec, Uint8*& buffer, Uint32& length) {
    if (SDL_LoadWAV(filename, &spec, &buffer, &length) == nullptr) {
        return false;
    }
    return true;
}

bool startAudio(SDL_AudioSpec& desiredSpec, Uint8* wavBuffer, Uint32 wavLength) {
    // Prepare desired audio spec (default to 44100 Hz mono S16)
    SDL_AudioSpec spec{};
    spec.freq = static_cast<int>(SAMPLE_RATE);
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = 8192;            // buffer size
    spec.callback = SDLAudioCallback;
    spec.userdata = nullptr;

    if (wavBuffer != nullptr && wavLength > 0) {
        // Loaded audio file – setup playback
        audioData = wavBuffer;
        audioDataLen = wavLength;
        audioChannels = desiredSpec.channels;
        // Use the file's format/frequency if needed
        spec.freq = desiredSpec.freq;
        spec.format = desiredSpec.format;
        spec.channels = desiredSpec.channels;
        // Calculate total frames (sample frames) in the audio data
        totalFrames = audioDataLen / (sizeof(int16_t) * audioChannels);
        frameIndex = 0;
        totalSamplesGenerated = 0;  // not used in file mode
    } else {
        // No file loaded – will generate tone
        audioData = nullptr;
        audioDataLen = 0;
        audioChannels = 1;
        totalFrames = 0;
        frameIndex = 0;
        totalSamplesGenerated = 0;
    }

    if (SDL_OpenAudio(&spec, nullptr) < 0) {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        if (wavBuffer) SDL_FreeWAV(wavBuffer);
        SDL_Quit();
        return false;
    }
    SDL_PauseAudio(0);  // start playback
    return true;
}

void closeAudio() {
    SDL_CloseAudio();
    if (audioData != nullptr) {
        SDL_FreeWAV(audioData);
        audioData = nullptr;
    }
}

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Try loading an audio file (optional)
    SDL_AudioSpec wavSpec;
    Uint8* wavBuffer = nullptr;
    Uint32 wavLength = 0;
    if (!loadWavFile("audio.wav", wavSpec, wavBuffer, wavLength)) {
        std::cerr << "No audio file found. Generating tone instead.\n";
    }

    if (!startAudio(wavSpec, wavBuffer, wavLength)) {
        // Audio initialization failed (error already logged)
        return 1;
    }

    // Create the application window and renderer
    SDL_Window* window = SDL_CreateWindow("Audio Visualization",
                                         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                         800, 600, 0);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        quitFlag = true;
        closeAudio();
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
                                               SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        quitFlag = true;
        closeAudio();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Main loop: handle events and draw waveform
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                // Window closed by user
                quitFlag.store(true);
                running = false;
            }
            // (Additional event handling like key presses can go here)
        }

        // Clear screen and draw waveform
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);
        int centerY = h / 2;
        int x = 0;
        int prevY = centerY;
        int16_t sample;
        while (audioRingBuffer.pop(sample) && x < w) {
            int y = centerY - (sample * centerY / 32767);
            if (x > 0) {
                SDL_RenderDrawLine(renderer, x - 1, prevY, x, y);
            }
            prevY = y;
            ++x;
        }

        SDL_RenderPresent(renderer);
        // (With SDL_PRESENTVSYNC, the loop will sync to the monitor’s refresh)
    }

    // Cleanup resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    closeAudio();
    SDL_Quit();
    return 0;
}
