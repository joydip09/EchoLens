#pragma once

#include <cstddef>
#include <cstdint>

#include <atomic>

// Thread-safe circular buffer for 16-bit PCM audio samples.
//
// Producer (audio task) and consumer (network task) run on different
// FreeRTOS tasks. This is a single-producer/single-consumer lock-free
// buffer: neither hot path waits for a mutex. Capacity is fixed at
// construction time and backed by one startup allocation.
//
// This class knows nothing about I2S, WebSockets, or Deepgram. It is a
// generic byte-oriented mechanism reused wherever a producer/consumer
// pair needs decoupling.

namespace echolens::system {

class AudioRingBuffer {
public:
    explicit AudioRingBuffer(size_t capacitySamples);
    ~AudioRingBuffer();

    AudioRingBuffer(const AudioRingBuffer&) = delete;
    AudioRingBuffer& operator=(const AudioRingBuffer&) = delete;

    // Writes an entire frame or drops it. Returning zero means the frame was
    // dropped because there was insufficient space; callers can report this
    // later through takeDroppedSamples() without blocking the audio task.
    size_t write(const int16_t* samples, size_t count);

    // Reads up to `count` samples into `out`. Returns the number actually
    // read; returns 0 (no blocking) if fewer samples than requested are
    // available.
    size_t read(int16_t* out, size_t count);

    size_t availableToRead() const;
    size_t freeSpace() const;
    size_t capacity() const { return capacity_; }
    size_t takeDroppedSamples();

private:
    int16_t* buffer_;
    size_t capacity_;
    std::atomic<size_t> head_{0};  // monotonically increasing write sequence
    std::atomic<size_t> tail_{0};  // monotonically increasing read sequence
    std::atomic<size_t> droppedSamples_{0};
};

}  // namespace echolens::system
