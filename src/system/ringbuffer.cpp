#include "ringbuffer.h"

namespace echolens::system {

AudioRingBuffer::AudioRingBuffer(size_t capacitySamples)
    : buffer_(new int16_t[capacitySamples]), capacity_(capacitySamples) {}

AudioRingBuffer::~AudioRingBuffer() {
    delete[] buffer_;
}

size_t AudioRingBuffer::write(const int16_t* samples, size_t count) {
    if (samples == nullptr || count == 0 || count > capacity_) {
        return 0;
    }

    const size_t head = head_.load(std::memory_order_relaxed);
    const size_t tail = tail_.load(std::memory_order_acquire);
    if (head - tail + count > capacity_) {
        droppedSamples_.fetch_add(count, std::memory_order_relaxed);
        return 0;
    }

    for (size_t index = 0; index < count; ++index) {
        buffer_[(head + index) % capacity_] = samples[index];
    }
    head_.store(head + count, std::memory_order_release);
    return count;
}

size_t AudioRingBuffer::read(int16_t* out, size_t count) {
    if (out == nullptr || count == 0) {
        return 0;
    }
    const size_t tail = tail_.load(std::memory_order_relaxed);
    const size_t head = head_.load(std::memory_order_acquire);
    const size_t available = head - tail;
    const size_t toRead = count < available ? count : available;
    for (size_t index = 0; index < toRead; ++index) {
        out[index] = buffer_[(tail + index) % capacity_];
    }
    tail_.store(tail + toRead, std::memory_order_release);
    return toRead;
}

size_t AudioRingBuffer::availableToRead() const {
    return head_.load(std::memory_order_acquire) - tail_.load(std::memory_order_acquire);
}

size_t AudioRingBuffer::freeSpace() const {
    return capacity_ - availableToRead();
}

size_t AudioRingBuffer::takeDroppedSamples() {
    return droppedSamples_.exchange(0, std::memory_order_acq_rel);
}

}  // namespace echolens::system
