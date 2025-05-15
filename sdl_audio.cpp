#include <SDL2/SDL.h>
// #include "sound.h"
// using sound::WaveType;
#include "sound_modular.h"
using sound_modular::ModularVoice;
#include "shared_buffer.h"
#include "Filtered_Feedback_Drive.h"
#include "delay.h" // 🎯 Include our delay effect
#include "reverb.h"
#include "chorus.h"
#include "Moog_Filter.h"
#include "Filtered_Feedback_Drive.h"

#include "lfo_engine.h"
#include "lfo_ui.h"
#include "arp.h"
// maybe delete this below line you figure it out.
#include "arp_ui.h" // ✅ make sure this is at the top

#include <atomic>
#include <cmath>         // for std::pow
#include <algorithm>     // for std::clamp
#include "adsr_engine.h" // 💙 New ADSR engine integration

static FilteredFeedbackDrive filteredFeedbackDrive(SAMPLE_RATE);

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
void StartOscilloscope(SDL_Renderer *renderer);

// Buffer size for SDL audio callback
const int BUFFER_SIZE = 8192;
static uint64_t totalSamples = 0; // Continuous sample counter

static ModularVoice padVoice;
static ModularVoice seqVoice;

// 🎯 Global delay effect — 44100 = 1 second buffer, 500ms default delay
static Delay delayEffect(SAMPLE_RATE, 1000);
static Reverb reverb(44100.0); // ✅ Now matches constructor
static Chorus chorus(44100.0); // 👈 you might already have this

// Audio callback function for SDL
void AudioCallback(void *userdata, Uint8 *stream, int len)
{
    int16_t *buffer = reinterpret_cast<int16_t *>(stream);
    int samples = len / sizeof(int16_t);

    // 🍀 Apply delay parameters from UI
    delayEffect.setDelayTime(static_cast<int>(delayTime.load()));
    delayEffect.setFeedback(delayFeedback.load());
    delayEffect.setMix(delayMix.load());

    // 🎯 Step sequencer state
    static int stepIndex = 0;
    static int stepCounter = 0;

    int bpm = BPM.load();
    int stepLength = static_cast<int>((SAMPLE_RATE * 60.0) / (bpm * 4));     // samples per step
    int keyMidiNote = 60 + scaleBank.getSelectedKey();                       // C4 = 60, G = 67, etc.
    double baseFrequency = 440.0 * std::pow(2.0, (keyMidiNote - 69) / 12.0); // A4 = 440 Hz

    static float feedbackLoop = 0.0f;

    for (int i = 0; i < samples; ++i)
    {   
        // TEST DELETE IF IT WORKS.
        // float feedbackLoop = 0.0f; // 🔁 store feedback sample for reinjection

        // 🎯 Step advancement
        if (stepCounter++ >= stepLength)
        {
            stepCounter = 0;
            stepIndex = (stepIndex + 1) % 16;

            int note = 60;
            double freq = 440.0 * std::pow(2.0, (note - 69) / 12.0); // MIDI note → Hz

            if (stepSequence[stepIndex])
            {
                int pitchOffset = stepPitches[stepIndex].load();
                double freq = baseFrequency * std::pow(2.0, pitchOffset / 12.0);
                freq = scaleBank.applyScale(freq); // ✅ quantize to selected scale/key

                seqVoice.active = true;
                seqVoice.time = 0.0;
                seqVoice.duration = stepLength / static_cast<double>(SAMPLE_RATE);
                seqVoice.frequency = freq;

                // Setup ADSR properly
                seqVoice.ampEnv = sound_modular::ADSR{
                    uiAttackTime.load(),
                    uiDecayTime.load(),
                    uiSustainLevel.load(),
                    uiReleaseTime.load()};
                seqVoice.ampEnv.noteOn();

                // Setup oscillator levels
                seqVoice.sawLevel = oscSawLevel.load();
                seqVoice.squareLevel = oscSquareLevel.load();
                seqVoice.sineLevel = oscSineLevel.load();
                seqVoice.subLevel = oscSubLevel.load();
                seqVoice.volume = oscVolume.load();

                seqVoice.pwmAmount = oscPWMAmount.load();
                seqVoice.metalAmount = oscMetalizerAmount.load();
                seqVoice.ultrasawAmount = oscUltrasawAmount.load();
                seqVoice.saturationAmount = oscSaturationAmount.load();
                seqVoice.volume = oscVolume.load();
            }
        }

        Arp_Update(); // ✅ Always step the arpeggiator every sample

        // 🌹 Corrected Pad Note Handling
        NoteEvent evt;
        if (padNoteEvents.pop(evt))
        {

            if (evt.frequency < 0 && padVoice.active)
            {
                padVoice.ampEnv.noteOff(); // 🌹 Trigger noteOff properly
            }
            else
            {
                // Apply scale to the note
                // double scaledFrequency = scaleBank.applyScale(evt.frequency);

                padVoice.active = true;
                padVoice.time = 0.0;
                padVoice.duration = evt.duration;
                padVoice.frequency = evt.frequency;
                padVoice.ampEnv = sound_modular::ADSR{
                    uiAttackTime.load(),
                    uiDecayTime.load(),
                    uiSustainLevel.load(),
                    uiReleaseTime.load()};
                padVoice.sawLevel = oscSawLevel.load();
                padVoice.squareLevel = oscSquareLevel.load();
                padVoice.sineLevel = oscSineLevel.load();
                padVoice.subLevel = oscSubLevel.load();
                padVoice.volume = oscVolume.load();
                padVoice.pwmAmount = oscPWMAmount.load();
                padVoice.metalAmount = oscMetalizerAmount.load();
                padVoice.ultrasawAmount = oscUltrasawAmount.load();
                padVoice.saturationAmount = oscSaturationAmount.load();
                padVoice.volume = oscVolume.load();

                //         printf("PWM: %.2f Metal: %.2f Ultra: %.2f Noise: %.2f\n",
                //    oscPWMAmount.load(), oscMetalizerAmount.load(),
                //    oscUltrasawAmount.load(), oscNoiseAmount.load());

                padVoice.ampEnv.noteOn();
            }
        }
        // Auto-release sequencer voice when step duration ends
        if (seqVoice.active &&
            seqVoice.time >= seqVoice.duration &&
            seqVoice.ampEnv.state != sound_modular::ADSR::Release &&
            seqVoice.ampEnv.state != sound_modular::ADSR::Idle)
        {
            seqVoice.ampEnv.noteOff(); // ✅ Trigger release phase
        }


        // 💡 Modulate pitch using LFO if enabled
        // We sample the LFO once per sample frame (dt = 1 / SAMPLE_RATE)
        if (seqVoice.active)
        {
            ApplyLFOPitch(seqVoice.frequency);
        }
        if (padVoice.active)
        {
            ApplyLFOPitch(padVoice.frequency);
        }
        


        // 🍀 Get consistent dry sample (avoid double-advancing envelope)
        int seqSample = seqVoice.active ? seqVoice.getSample() : 0;
        int padSample = padVoice.active ? padVoice.getSample() : 0;
        
        int sample = seqSample + padSample;
        float drySample = sample / 32768.0f;

        ApplyLFOAmplitude(drySample);
        ApplyLFOFilter(drySample);
        // float moogOut = moogFilter.process(drySample + feedbackLoop); // Combined signal

        if (filteredFeedbackEnabled.load())
        {
            filteredFeedbackDrive.setDrive(overdriveDrive.load());
            filteredFeedbackDrive.setCutoff(steinerCutoff.load());
            filteredFeedbackDrive.setResonance(steinerResonance.load());
            filteredFeedbackDrive.setVcaLevel(steinerVcaLevel.load());
            filteredFeedbackDrive.setFeedbackAmount(feedbackAmount.load());
            // 🎧 Create feedback loop
            float combinedInput = drySample + feedbackLoop;
            float moogOut = moogFilter.process(combinedInput);
            float driven = filteredFeedbackDrive.processSample(moogOut);

            // 🌀 Store for next pass (scaled by feedbackAmount)
            feedbackLoop = driven * feedbackAmount.load();

            // ✅ Output *still starts* from the clean dry input
            
            // drySample = combinedInput;  
            drySample = driven;  // ✅ ACTUALLY USE FFD OUTPUT

            // or just leave drySample untouched
            
        }
        else
        {
            // drySample = moogOut; // skip FFD
            drySample = moogFilter.process(drySample);
        }
    

    // 🎛️ Apply Chorus (if implemented)
    if (chorusEnabled.load())
    {
        chorus.setRate(chorusRate.load());
        chorus.setDepth(chorusDepth.load());
        chorus.setMix(chorusMix.load());
        drySample = chorus.process(drySample);
    }

    // Apply Delay
    if (delayEnabled.load())
    {
        delayEffect.setDelayTime(static_cast<int>(delayTime.load()));
        delayEffect.setFeedback(delayFeedback.load());
        delayEffect.setMix(delayMix.load());
        drySample = delayEffect.process(drySample);
    }

    // 🌫️ Apply Reverb
    if (reverbEnabled.load())
    {
        reverb.setDecay(reverbDecay.load());
        reverb.setDamping(reverbDamping.load());
        reverb.setMix(reverbMix.load());
        reverb.setPreDelay(reverbPreDelay.load()); // 🎸 new: set pre-delay from control
        drySample = reverb.process(drySample);
    }

    // ✅ Final sample conversion
    int finalSample = static_cast<int>(drySample * 32768.0f);

    // Clip and write
    sample = std::clamp(sample, -32768, 32767);
    finalSample = std::clamp(finalSample, -32768, 32767);    // ✅ Use final mixed value
    buffer[i] = static_cast<int16_t>(finalSample);           // ✅ Write correct sample
    audioRingBuffer.push(static_cast<int16_t>(finalSample)); // 🍀 Restore oscilloscope

    totalSamples++;
}
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) < 0)
    {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
        return -1;
    }

    ArpUI_Init(); // ✅ Inject test notes

    SDL_AudioSpec spec;
    spec.freq = SAMPLE_RATE;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.samples = BUFFER_SIZE;
    spec.callback = AudioCallback;
    spec.userdata = nullptr;

    if (SDL_OpenAudio(&spec, nullptr) < 0)
    {
        SDL_Log("Failed to open audio: %s", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    ArpUI_Init(); // ✅ Inject test notes

    SDL_Window *window = SDL_CreateWindow("Oscilloscope",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          800, 400, SDL_WINDOW_SHOWN);

    if (!window)
    {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        SDL_CloseAudio();
        SDL_Quit();
        return -1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!renderer)
    {
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
