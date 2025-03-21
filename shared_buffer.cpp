#include "shared_buffer.h"
#include "sound.h"

bool stepSequence[16] = {0};  // All OFF initially

RingBuffer<int16_t> audioRingBuffer(16384);

// Define the waveform selector correctly
std::atomic<sound::WaveType> currentWaveform = sound::WaveType::SINE;