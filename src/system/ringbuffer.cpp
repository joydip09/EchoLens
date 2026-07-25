#include "ringbuffer.h"

#include <algorithm>
#include <cstring>

#include "logger.h"

namespace echolens::system {

namespace {
constexpr const char* kTag = "RingBuffer";
constexpr TickType_t kLockTimeout = pdMS_TO_TICKS(20);
}  // namespace

AudioRingBuffer::AudioRingBuffer(size_t capacitySamples)
    : buffer_(new int16_t[capacitySamples]), capacity_(capacitySamples) {
    mutex_ = xSemaphoreCreateMutex();
}

AudioRingBuffer::~AudioRingBuffer() {
    delete[] buffer_;
    if (mutex_ != nullptr) {
        vSemaphoreDelete(mutex_);
    }
}

size_t AudioRingBuffer::write(const int16_t* samples, size_t count) {
    if (samples == nullptr || count == 0) {
        return 0;
    }
    if (xSemaphoreTake(mutex_, kLockTimeout) != pdTRUE) {
        return 0;
    }

    // Never attempt to write more than the buffer can physically hold in
    // one pass; the caller is expected to chunk larger sources.
    size_t toWrite = std::min(count, capacity_);

    // Overflow protection: if writing would exceed capacity, advance the
    // read tail to drop the oldest samples rather than corrupt state.
    size_t spaceAvailable = capacity_ - count_;
    if (toWrite > spaceAvailable) {
        size_t overflow = toWrite - spaceAvailable;
        tail_ = (tail_ + overflow) % capacity_;
        count_ -= overflow;
        Logger::warning(kTag, "Overrun: dropped %u samples", static_cast<unsigned>(overflow));
    }

    for (size_t i = 0; i < toWrite; ++i) {
        buffer_[head_] = samples[i];
        head_ = (head_ + 1) % capacity_;
    }
    count_ += toWrite;

    xSemaphoreGive(mutex_);
    return toWrite;
}

size_t AudioRingBuffer::read(int16_t* out, size_t count) {
    if (out == nullptr || count == 0) {
        return 0;
    }
    if (xSemaphoreTake(mutex_, kLockTimeout) != pdTRUE) {
        return 0;
    }

    // Underflow protection: never read more than what is actually stored.
    size_t toRead = std::min(count, count_);
    for (size_t i = 0; i < toRead; ++i) {
        out[i] = buffer_[tail_];
        tail_ = (tail_ + 1) % capacity_;
    }
    count_ -= toRead;

    xSemaphoreGive(mutex_);
    return toRead;
}

size_t AudioRingBuffer::availableToReadLocked() const {
    return count_;
}

size_t AudioRingBuffer::availableToRead() const {
    size_t result = 0;
    if (xSemaphoreTake(mutex_, kLockTimeout) == pdTRUE) {
        result = count_;
        xSemaphoreGive(mutex_);
    }
    return result;
}

size_t AudioRingBuffer::freeSpace() const {
    size_t result = 0;
    if (xSemaphoreTake(mutex_, kLockTimeout) == pdTRUE) {
        result = capacity_ - count_;
        xSemaphoreGive(mutex_);
    }
    return result;
}

}  // namespace echolens::system
