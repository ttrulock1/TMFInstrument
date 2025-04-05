#include <SDL2/SDL.h>
#include "sound.h"
using sound::WaveType;
#include "shared_buffer.h"
#include "delay.h"  // 🎯 Include our delay effect
#include <atomic>
#include <cmath>  // for std::pow
#include <algorithm> // for std::clamp

// 🎯 Bring in externally declared ADSR control parameters
extern std::atomic<float> attackTime;
extern std::atomic<float> decayTime;
extern std::atomic<float> sustainLevel;
extern std::atomic<float> releaseTime;
// ** doesn't this get declare somewhere else or is it needed in both places
extern std::atomic<float> delayTime;
extern std::atomic<float> delayFeedback;
extern std::atomic<float> delayMix;
extern std::atomic<bool> delayEnabled;

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
    if (time >= duration) {
        // 🍀 Softly fade sample tail instead of hard cutoff
        s = static_cast<short>(s * 0.9); // exponential decay
        // 🍀 Only deactivate when sample is basically silent
        if (std::abs(s) < 128)
            active = false;
    }
    time += 1.0 / SAMPLE_RATE;
    return s;
}

};

// 🎯 Two active voices (duophonic)
static Voice padVoice;
static Voice seqVoice;

// 🎯 Global delay effect — 44100 = 1 second buffer, 500ms default delay
static Delay delayEffect(SAMPLE_RATE, 1000);

// Audio callback function for SDL
void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    // 🍀 Apply delay parameters from UI
    delayEffect.setDelayTime(static_cast<int>(delayTime.load()));
    delayEffect.setFeedback(delayFeedback.load());
    delayEffect.setMix(delayMix.load());

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
        if (padNoteEvents.pop(evt)) {
            if (evt.frequency < 0 && padVoice.active) {
                padVoice.duration = padVoice.time + releaseTime.load(); 
            } else {
                padVoice = Voice{
                    .active = true,
                    .time = 0.0,
                    .duration = evt.duration,
                    .frequency = evt.frequency,
                    .env = evt.env,
                    .wave = currentWaveform.load()
                };
            }
        }

        // 🍀 Get consistent dry sample (avoid double-advancing envelope)
        int seqSample = seqVoice.active ? seqVoice.getSample() : 0;
        int padSample = padVoice.active ? padVoice.getSample() : 0;
        int sample = seqSample + padSample;
        float drySample = sample / 32768.0f;

        // 🍀 Conditionally apply delay
        float wetSample = delayEnabled.load()
            ? delayEffect.process(drySample)
            : drySample;

        float mixed = drySample + wetSample * 0.5f;
        int finalSample = static_cast<int>(mixed * 32768.0f);

        // Clip and write
        sample = std::clamp(sample, -32768, 32767);
        finalSample = std::clamp(finalSample, -32768, 32767);  // ✅ Use final mixed value
        buffer[i] = static_cast<int16_t>(finalSample);         // ✅ Write correct sample
        audioRingBuffer.push(static_cast<int16_t>(finalSample));  // 🍀 Restore oscilloscope

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
