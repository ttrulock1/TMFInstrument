#include "shared_buffer.h"

// Create a ring buffer for audio samples with a capacity of 16384 samples.
RingBuffer<int16_t> audioRingBuffer(16384);
