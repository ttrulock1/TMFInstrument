#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include "ring_buffer.h"
#include <cstdint>
#include <atomic>
#include "sound.h"  // Include sound::WaveType definition

extern RingBuffer<int16_t> audioRingBuffer;
extern bool stepSequence[16];  // Step on/off states for the 16 steps

// Current waveform type (atomic for thread-safety between UI and audio threads)
extern std::atomic<sound::WaveType> currentWaveform;


// Pitch offset for each step (in semitones, range -12 to +12; 0 = base pitch)
extern std::atomic<int> stepPitches[16];

extern std::atomic<int> BPM;  // Beats per minute (atomic for thread-safety)

struct NoteEvent {
    double frequency;
    double duration;
    sound::ADSR env;
};

extern RingBuffer<NoteEvent> padNoteEvents; // clearly for pad notes only

// 🌊 Chorus parameters
extern std::atomic<float> chorusRate;
extern std::atomic<float> chorusDepth;
extern std::atomic<float> chorusMix;
extern std::atomic<bool> chorusEnabled;

// 👽 Delay parameters (used in effects.cpp, sdl_audio.cpp)
extern std::atomic<float> delayTime;
extern std::atomic<float> delayFeedback;
extern std::atomic<float> delayMix;
extern std::atomic<float> delayHighCut;
extern std::atomic<bool> delayEnabled;

// shared_buffer.h
// 👽 Reverb parameters 
extern std::atomic<float> reverbDecay;
extern std::atomic<float> reverbDamping;
extern std::atomic<float> reverbMix;
extern std::atomic<bool> reverbEnabled;


#endif // SHARED_BUFFER_H
