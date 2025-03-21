#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include "ring_buffer.h"
#include <cstdint>
#include <atomic>
#include "sound.h"  // here to access sound::WaveType

extern RingBuffer<int16_t> audioRingBuffer;
extern bool stepSequence[16];

// 🔹 Correct declaration with sound::WaveType
extern std::atomic<sound::WaveType> currentWaveform;

extern std::atomic<int> BPM;

#endif // SHARED_BUFFER_H
