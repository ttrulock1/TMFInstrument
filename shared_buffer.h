#ifndef SHARED_BUFFER_H
#define SHARED_BUFFER_H

#include "ring_buffer.h"
#include <cstdint>

// Declare a global ring buffer for audio samples.
extern RingBuffer<int16_t> audioRingBuffer;

#endif // SHARED_BUFFER_H
