#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "../../include/config.h"
#include "parser.h"
#include "../system/websocket_transport.h"

// Provider abstraction: the rest of the firmware talks to a
// TranscriptionProvider interface, never to Deepgram directly. Swapping
// in AssemblyAI, Groq, or a local Whisper server later means adding a new
// subclass, not touching main.cpp or the audio/display pipeline.

namespace echolens::transcription {

using TranscriptCallback = std::function<void(const TranscriptResult&)>;

class TranscriptionProvider {
public:
    virtual ~TranscriptionProvider() = default;

    virtual void begin() = 0;
    virtual void poll() = 0;
    virtual bool isConnected() const = 0;

    // Streams a chunk of mono 16-bit PCM audio upstream.
    virtual bool sendAudio(const int16_t* samples, size_t sampleCount) = 0;

    virtual void onTranscript(TranscriptCallback callback) = 0;
};

class DeepgramProvider : public TranscriptionProvider {
public:
    DeepgramProvider(const DeepgramConfig& config, system::WebSocketTransport& socket);

    void begin() override;
    void poll() override;
    bool isConnected() const override;
    bool sendAudio(const int16_t* samples, size_t sampleCount) override;
    void onTranscript(TranscriptCallback callback) override;

private:
    void handleMessage(const uint8_t* data, size_t length, bool isBinary);
    void handleConnectionChange(bool connected);

    DeepgramConfig config_;
    system::WebSocketTransport& socket_;
    TranscriptCallback transcriptCallback_;
    std::string authHeader_;
};

}  // namespace echolens::transcription
