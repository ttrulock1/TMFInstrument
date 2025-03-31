#include <SDL2/SDL.h>
#include "sound.h"
using sound::WaveType;
#include "shared_buffer.h"
#include <atomic>
#include <cmath>  // for std::pow function


// Forward declare visualization function
void StartOscilloscope(SDL_Renderer* renderer);

// Buffer size for SDL audio callback
const int BUFFER_SIZE = 8192;
static uint64_t totalSamples = 0;  // Continuous sample counter

// Audio callback function for SDL
void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    // Step sequencer state
    static int stepIndex = 0;
    static int stepCounter = 0;

    // Calculate step length based on current BPM (dynamic step timing)
    int bpm = BPM.load();
    int stepLength = static_cast<int>((SAMPLE_RATE * 60.0) / (bpm * 4));  // samples per step

    // Synthesis parameters
    double baseFrequency = 440.0;      // Base frequency (A4 = 440 Hz)
    double amp = 0.5;                  // Amplitude
    sound::ADSR env = {0.01, 0.1, 0.8, 0.1};  // Envelope (attack, decay, sustain, release)
    WaveType waveType = currentWaveform.load();  // Current waveform selection
    double duration = 1.0;             // Note duration for envelope (not heavily used here)

    // Generate audio samples

    
    for (int i = 0; i < samples; ++i) {
        // Advance step timing
        if (stepCounter++ >= stepLength) {
            stepCounter = 0;
            stepIndex = (stepIndex + 1) % 16;  // move to the next step (wrap around 16 steps)
        }

        // If the current step is active, generate a tone with the step's pitch offset
        if (stepSequence[stepIndex]) {
            // Compute frequency for this step by applying the pitch offset (semitones) to the base frequency
            int pitchOffset = stepPitches[stepIndex].load();
            double stepFrequency = baseFrequency * std::pow(2.0, pitchOffset / 12.0);  
            // ^ Formula: freq = baseFreq * 2^(pitchOffset/12)

            // Generate the waveform sample at the calculated frequency
            double time = static_cast<double>(totalSamples++) / SAMPLE_RATE;
            buffer[i] = sound::GenerateWave(waveType, time, duration, stepFrequency, amp, env);
        } else {
            // Step is off; output silence
            buffer[i] = 0;
            totalSamples++;  // still advance the global sample counter to keep time continuity
        }

        // Push sample to the ring buffer for visualization (oscilloscope)
        audioRingBuffer.push(buffer[i]);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // Setup audio spec for desired output
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

    // Create a window for oscilloscope display (visualization)
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

    // Start audio playback and visualization loop
    SDL_PauseAudio(0);            // Begin audio callback processing (unpause audio)
    StartOscilloscope(renderer);  // Enter visualization loop until quit

    // Cleanup on exit
    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
