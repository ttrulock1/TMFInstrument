#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <atomic>
#include <vector>
#include <cstddef>

template<typename T>
class RingBuffer {
public:
    RingBuffer(size_t size) : buffer(size), size(size), writeIndex(0), readIndex(0) {}

    bool push(const T& value) {
        size_t currentWrite = writeIndex.load(std::memory_order_relaxed);
        size_t nextWrite = (currentWrite + 1) % size;
        if (nextWrite == readIndex.load(std::memory_order_acquire)) {
            // Buffer is full.
            return false;
        }
        buffer[currentWrite] = value;
        writeIndex.store(nextWrite, std::memory_order_release);
        return true;
    }

    bool pop(T& value) {
        size_t currentRead = readIndex.load(std::memory_order_relaxed);
        if (currentRead == writeIndex.load(std::memory_order_acquire)) {
            // Buffer is empty.
            return false;
        }
        value = buffer[currentRead];
        readIndex.store((currentRead + 1) % size, std::memory_order_release);
        return true;
    }

private:
    std::vector<T> buffer;
    const size_t size;
    std::atomic<size_t> writeIndex;
    std::atomic<size_t> readIndex;
};

#endif // RING_BUFFER_H
