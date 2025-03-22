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

#endif // SHARED_BUFFER_H
