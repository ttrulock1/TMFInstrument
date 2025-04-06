#include "shared_buffer.h"
#include "sound.h"

bool stepSequence[16] = {0};  // All steps OFF initially

RingBuffer<int16_t> audioRingBuffer(16384);

// Define the pitch offset array for 16 steps (initially all zeros = no pitch shift)
std::atomic<int> stepPitches[16];  // Defaults to 0 semitone offset for each step

// Define the current waveform selector (initialized to SINE wave)
std::atomic<sound::WaveType> currentWaveform = sound::WaveType::SINE;

// Define BPM (can be modified elsewhere)
std::atomic<int> BPM = 120;  // default BPM, for example

std::atomic<float> attackTime = 0.01f;
std::atomic<float> decayTime = 0.1f;
std::atomic<float> sustainLevel = 0.8f;
std::atomic<float> releaseTime = 0.1f;


//✅ These become the actual backing variables that your effects UI and sdl_audio.cpp will modify.

// 👽 Delay effect parameter definitions
std::atomic<float> delayTime = 0.25f;         // 0.0 - 1.0
std::atomic<float> delayFeedback = 0.4f;      // 0.0 - 1.0
std::atomic<float> delayMix = 0.3f;           // 0.0 - 1.0
std::atomic<float> delayHighCut = 1.0f;       // normalized 0.0 to 1.0 (placeholder)
std::atomic<bool> delayEnabled = true;

// 🌊 Chorus
std::atomic<float> chorusRate = 0.5f;
std::atomic<float> chorusDepth = 0.6f;
std::atomic<float> chorusMix = 0.25f;
std::atomic<bool> chorusEnabled = true;

// 🌫️ Reverb
std::atomic<float> reverbDecay = 2.0f;
std::atomic<float> reverbDamping = 0.5f;
std::atomic<float> reverbMix = 0.3f;
std::atomic<bool> reverbEnabled = true;



RingBuffer<NoteEvent> padNoteEvents(128); // small queue for instant pad notes
