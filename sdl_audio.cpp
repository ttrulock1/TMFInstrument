#include <SDL2/SDL.h>
#include "sound.h"
using sound::WaveType;        // Brings WaveType into scope so we can use it directly
#include "shared_buffer.h"
#include <atomic>


// Forward declare visualization function
void StartOscilloscope(SDL_Renderer* renderer);

// Buffer size for SDL audio callback.
const int BUFFER_SIZE = 8192;
static uint64_t totalSamples = 0;  // Continuous sample counter

// 🔹 Global BPM variable (linked from sdl_visual)
extern std::atomic<int> BPM;

void AudioCallback(void* userdata, Uint8* stream, int len) {
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    // Step Sequencer Variables
    static int stepIndex = 0;
    static int stepCounter = 0;

    // 🔹 Dynamic step length based on BPM
    int bpm = BPM.load(); 
    int stepLength = (int)((SAMPLE_RATE * 60.0) / (bpm * 4)); // BPM-adjusted step timing

    // Synthesis parameters
    double frequency = 440.0;  // Default frequency (A4)
    double amp = 0.5;
    sound::ADSR env = { 0.01, 0.1, 0.8, 0.1 };
    WaveType waveType = currentWaveform.load(); // this is the live selected waveform
    double duration = 1.0;

    for (int i = 0; i < samples; ++i) {
        // Step Sequencer Logic
        if (stepCounter++ >= stepLength) {
            stepCounter = 0;
            stepIndex = (stepIndex + 1) % 16;  // Move to next step
        }

        // Only play sound if the step is active
        if (stepSequence[stepIndex]) {
            double time = static_cast<double>(totalSamples++) / SAMPLE_RATE;
            buffer[i] = sound::GenerateWave(waveType, time, duration, frequency, amp, env);
        } else {
            buffer[i] = 0;  // Silence if step is OFF
        }

        // Push audio data into ring buffer for visualization
        audioRingBuffer.push(buffer[i]);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    // Setup audio specification
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

    // Create window for oscilloscope display
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
    SDL_PauseAudio(0);                 // Begin audio callback processing
    StartOscilloscope(renderer);       // Run oscilloscope until user quits

    // Cleanup resources after loop exits
    SDL_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
