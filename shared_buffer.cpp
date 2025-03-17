#include "shared_buffer.h"

bool stepSequence[16] = {0};  // Default all steps OFF

// Create a ring buffer for audio samples with a capacity of 16384 samples.
RingBuffer<int16_t> audioRingBuffer(16384);
