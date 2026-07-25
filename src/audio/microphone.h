#pragma once

#include <cstddef>
#include <cstdint>

// Captures raw PCM audio from an INMP441 I2S microphone.
//
// This module knows NOTHING about WiFi, WebSockets, Deepgram, or the
// OLED. Its only job is: configure the I2S peripheral and hand back
// signed 16-bit mono samples. Where those samples go is the caller's
// problem.

namespace echolens::audio {

struct MicrophoneConfig {
    int bclkPin;
    int wsPin;
    int dataInPin;
    uint32_t sampleRateHz;
};

class Microphone {
public:
    explicit Microphone(const MicrophoneConfig& config);

    // Configures and starts the I2S peripheral in RX mode. Returns false
    // on driver installation failure.
    bool begin();

    // Blocking (bounded by I2S DMA availability) read of up to
    // `maxSamples` mono int16 samples into `out`. Returns the number of
    // samples actually captured. Never allocates.
    size_t readSamples(int16_t* out, size_t maxSamples);

    void stop();

private:
    MicrophoneConfig config_;
    bool running_ = false;
};

}  // namespace echolens::audio
