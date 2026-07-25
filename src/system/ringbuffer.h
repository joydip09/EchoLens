#pragma once

#include <cstddef>
#include <cstdint>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Thread-safe circular buffer for 16-bit PCM audio samples.
//
// Producer (audio task) and consumer (network task) run on different
// FreeRTOS tasks; all access is serialized through a mutex. Capacity is
// fixed at construction time and backed by a single heap allocation made
// once at startup - no allocation happens on the hot read/write path.
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

    // Writes up to `count` samples. Returns the number actually written.
    // If the buffer would overflow, oldest unread samples are dropped to
    // make room (overrun protection favors freshness over history for a
    // live-transcription use case).
    size_t write(const int16_t* samples, size_t count);

    // Reads up to `count` samples into `out`. Returns the number actually
    // read; returns 0 (no blocking) if fewer samples than requested are
    // available.
    size_t read(int16_t* out, size_t count);

    size_t availableToRead() const;
    size_t freeSpace() const;
    size_t capacity() const { return capacity_; }

private:
    size_t availableToReadLocked() const;

    int16_t* buffer_;
    size_t capacity_;
    size_t head_ = 0;  // next write index
    size_t tail_ = 0;  // next read index
    size_t count_ = 0; // samples currently stored

    SemaphoreHandle_t mutex_;
};

}  // namespace echolens::system
