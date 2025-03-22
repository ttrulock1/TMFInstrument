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
