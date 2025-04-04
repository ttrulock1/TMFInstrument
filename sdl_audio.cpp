#include <SDL2/SDL.h>
#include "sound.h"
using sound::WaveType;
#include "shared_buffer.h"
#include <atomic>
#include <cmath>  // for std::pow
#include <algorithm> // for std::clamp

// 🎯 Bring in externally declared ADSR control parameters
extern std::atomic<float> attackTime;
extern std::atomic<float> decayTime;
extern std::atomic<float> sustainLevel;
extern std::atomic<float> releaseTime;

// Forward declare visualization function
void StartOscilloscope(SDL_Renderer* renderer);

// Buffer size for SDL audio callback
const int BUFFER_SIZE = 8192;
static uint64_t totalSamples = 0;  // Continuous sample counter

// 🎯 Voice struct: reusable note-in-progress
struct Voice {
    bool active = false;
    double time = 0.0;
    double duration = 0.0;
    double frequency = 440.0;
    sound::ADSR env;
    WaveType wave;

    short getSample() {
        short s = sound::GenerateWave(wave, time, duration, frequency, 0.5, env);
        time += 1.0 / SAMPLE_RATE;
        if (time >= duration) active = false;
        return s;
    }
};

// 🎯 Two active voices (duophonic)
static Voice padVoice;
static Voice seqVoice;

// Audio callback function for SDL
void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    // 🎯 Step sequencer state
    static int stepIndex = 0;
    static int stepCounter = 0;

    int bpm = BPM.load();
    int stepLength = static_cast<int>((SAMPLE_RATE * 60.0) / (bpm * 4));  // samples per step
    double baseFrequency = 440.0;

    for (int i = 0; i < samples; ++i) {
        // 🎯 Step advancement
        if (stepCounter++ >= stepLength) {
            stepCounter = 0;
            stepIndex = (stepIndex + 1) % 16;

            if (stepSequence[stepIndex]) {
                int pitchOffset = stepPitches[stepIndex].load();
                double freq = baseFrequency * std::pow(2.0, pitchOffset / 12.0);
                seqVoice = Voice{
                    .active = true,
                    .time = 0.0,
                    .duration = stepLength / static_cast<double>(SAMPLE_RATE),
                    .frequency = freq,
                    .env = {
                        attackTime.load(),
                        decayTime.load(),
                        sustainLevel.load(),
                        releaseTime.load()
                    },
                    .wave = currentWaveform.load()
                };
            }
        }

        // 🎯 Handle incoming pad notes
        NoteEvent evt;
        if (!padVoice.active && padNoteEvents.pop(evt)) {
            padVoice = Voice{
                .active = true,
                .time = 0.0,
                .duration = evt.duration,
                .frequency = evt.frequency,
                .env = evt.env,
                .wave = currentWaveform.load()
            };
        }

        // 🎯 Mix both voices (duophonic!)
        int sample = 0;
        if (seqVoice.active) sample += seqVoice.getSample();
        if (padVoice.active) sample += padVoice.getSample();

        // Clip and write
        sample = std::clamp(sample, -32768, 32767);
        buffer[i] = static_cast<int16_t>(sample);
        audioRingBuffer.push(buffer[i]);
        totalSamples++;
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = BUFFER_SIZE;
    spec.callback = AudioCallback;
    spec.userdata = nullptr;

    if (SDL_OpenAudio(&spec, nullptr) < 0) {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Oscilloscope",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 400, SDL_WINDOW_SHOWN);

    if (!window) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_CloseAudio();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer) {
        SDL_Log("Failed to create renderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_CloseAudio();
        SDL_Quit();
        return -1;
    }

    SDL_PauseAudio(0);
    StartOscilloscope(renderer);

    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
