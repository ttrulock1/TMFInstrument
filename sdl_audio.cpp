#include <SDL2/SDL.h>
#include "sound.h"
using sound::WaveType;
#include "shared_buffer.h"
#include "delay.h"  // 🎯 Include our delay effect
#include "reverb.h"
#include "chorus.h"  // uncomment when created
#include "lfo_engine.h"
#include "lfo_ui.h"


#include <atomic>
#include <cmath>  // for std::pow
#include <algorithm> // for std::clamp
#include "adsr_engine.h" // 💙 New ADSR engine integration


// 🎯 Bring in externally declared ADSR control parameters
extern std::atomic<float> uiAttackTime;
extern std::atomic<float> uiDecayTime;
extern std::atomic<float> uiSustainLevel;
extern std::atomic<float> uiReleaseTime;
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
    ADSR ampEnv; // ✅ Now using ADSR engine
    WaveType wave;

short getSample() {
    float amplitude = ampEnv.process();  // 🌹 Correct ADSR process call
    short s = sound::GenerateWave(wave, time, frequency, amplitude); // ✅ Clean + correct
    if (ampEnv.state == ADSR::Idle)
        active = false;
    time += 1.0 / SAMPLE_RATE;
    return s;
}


};

// 🎯 Two active voices (duophonic)
static Voice padVoice;
static Voice seqVoice;

// 🎯 Global delay effect — 44100 = 1 second buffer, 500ms default delay
static Delay delayEffect(SAMPLE_RATE, 1000);
static Reverb reverb(44100.0); // ✅ Now matches constructor
static Chorus chorus(44100.0);  // 👈 you might already have this


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

            int note = 60;
            double freq = 440.0 * std::pow(2.0, (note - 69) / 12.0);  // MIDI note → Hz

            if (stepSequence[stepIndex]) {
                int pitchOffset = stepPitches[stepIndex].load();
                double freq = baseFrequency * std::pow(2.0, pitchOffset / 12.0);
                seqVoice = Voice{
                    .active = true,
                    .time = 0.0,
                    .duration = stepLength / static_cast<double>(SAMPLE_RATE),
                    .frequency = freq,
                    .ampEnv = ADSR{
                        uiAttackTime.load(),
                        uiDecayTime.load(),
                        uiSustainLevel.load(),
                        uiReleaseTime.load()
                    },
                    .wave = currentWaveform.load()
                };
                seqVoice.ampEnv.noteOn(); // 🌹 CRITICAL FIX


            }
        }

            // 🌹 Corrected Pad Note Handling
        NoteEvent evt;
        if (padNoteEvents.pop(evt)) {
            if (evt.frequency < 0 && padVoice.active) {
                padVoice.ampEnv.noteOff(); // 🌹 Trigger noteOff properly
            } else {
                padVoice = Voice{
                    .active = true,
                    .time = 0.0,
                    .duration = evt.duration,
                    .frequency = evt.frequency,
                    .ampEnv = ADSR{
                        uiAttackTime.load(),
                        uiDecayTime.load(),
                        uiSustainLevel.load(),
                        uiReleaseTime.load()
                    },                    
                    .wave = currentWaveform.load()
                };
                padVoice.ampEnv.noteOn(); // 🌹 Trigger noteOn properly
            }
        }
        // 🧨 Auto-release sequencer voice when step duration ends
if (seqVoice.active &&
    seqVoice.time >= seqVoice.duration &&
    seqVoice.ampEnv.state != ADSR::Release &&
    seqVoice.ampEnv.state != ADSR::Idle) {
    seqVoice.ampEnv.noteOff();  // ✅ Trigger release phase
}
// 💡 Modulate pitch using LFO if enabled
// We sample the LFO once per sample frame (dt = 1 / SAMPLE_RATE)
if (seqVoice.active) {
    ApplyLFOPitch(seqVoice.frequency);
}
if (padVoice.active) {
    ApplyLFOPitch(padVoice.frequency);
}


        // 🍀 Get consistent dry sample (avoid double-advancing envelope)
        int seqSample = seqVoice.active ? seqVoice.getSample() : 0;
        int padSample = padVoice.active ? padVoice.getSample() : 0;
        int sample = seqSample + padSample;
        float drySample = sample / 32768.0f;
        ApplyLFOAmplitude(drySample);
        ApplyLFOFilter(drySample);



        // // 🍀 Conditionally apply delay
        // float wetSample = delayEnabled.load()
        //     ? delayEffect.process(drySample)
        //     : drySample;

        // float mixed = drySample + wetSample * 0.5f;
        // int finalSample = static_cast<int>(mixed * 32768.0f);

        // 🔁 Apply effects in chain: Chorus → Delay → Reverb

// 🎛️ Apply Chorus (if implemented)
if (chorusEnabled.load()) {
    chorus.setRate(chorusRate.load());
    chorus.setDepth(chorusDepth.load());
    chorus.setMix(chorusMix.load());
    drySample = chorus.process(drySample);
}

// ⏱️ Apply Delay
if (delayEnabled.load()) {
    delayEffect.setDelayTime(static_cast<int>(delayTime.load()));
    delayEffect.setFeedback(delayFeedback.load());
    delayEffect.setMix(delayMix.load());
    // delayEffect.setHighCut(delayHighCut.load()); // if supported
    drySample = delayEffect.process(drySample);
}

// 🌫️ Apply Reverb
if (reverbEnabled.load()) {
    reverb.setDecay(reverbDecay.load());
    reverb.setDamping(reverbDamping.load());
    reverb.setMix(reverbMix.load());
    drySample = reverb.process(drySample);
}

// ✅ Final sample conversion
int finalSample = static_cast<int>(drySample * 32768.0f);


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
