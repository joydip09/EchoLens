#include "deepgram.h"

#include "../system/logger.h"

namespace echolens::transcription {

namespace {
constexpr const char* kTag = "Deepgram";
}  // namespace

DeepgramProvider::DeepgramProvider(const DeepgramConfig& config) : config_(config) {
    authHeader_ = std::string("Authorization: Token ") + config_.apiKey;
}

void DeepgramProvider::begin() {
    socket_.configure(config_.host, config_.port, config_.path, authHeader_.c_str());

    socket_.onMessage([this](const uint8_t* data, size_t length, bool isBinary) {
        handleMessage(data, length, isBinary);
    });

    socket_.onConnectionChange([this](bool connected) {
        handleConnectionChange(connected);
    });

    socket_.begin();
    system::Logger::info(kTag, "Provider initialized (host=%s)", config_.host);
}

void DeepgramProvider::poll() {
    socket_.poll();
}

bool DeepgramProvider::isConnected() const {
    return socket_.isConnected();
}

bool DeepgramProvider::sendAudio(const int16_t* samples, size_t sampleCount) {
    if (samples == nullptr || sampleCount == 0) {
        return false;
    }
    return socket_.sendBinary(reinterpret_cast<const uint8_t*>(samples),
                               sampleCount * sizeof(int16_t));
}

void DeepgramProvider::onTranscript(TranscriptCallback callback) {
    transcriptCallback_ = std::move(callback);
}

void DeepgramProvider::handleMessage(const uint8_t* data, size_t length, bool isBinary) {
    if (isBinary) {
        // Deepgram's streaming API only sends JSON text frames; a binary
        // frame here would be unexpected protocol drift.
        system::Logger::warning(kTag, "Unexpected binary frame (%u bytes)", static_cast<unsigned>(length));
        return;
    }

    TranscriptResult result = ParseDeepgramResponse(data, length);
    if (result.valid && transcriptCallback_) {
        transcriptCallback_(result);
    }
}

void DeepgramProvider::handleConnectionChange(bool connected) {
    if (connected) {
        system::Logger::info(kTag, "Session established");
    } else {
        system::Logger::warning(kTag, "Session lost, will auto-reconnect");
    }
}

}  // namespace echolens::transcription
