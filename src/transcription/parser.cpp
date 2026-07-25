#include "parser.h"

#include <cstring>

#include <ArduinoJson.h>

#include "../system/logger.h"

namespace echolens::transcription {

namespace {
constexpr const char* kTag = "Parser";
// Deepgram streaming payloads are small; a fixed-capacity document avoids
// heap churn on every message.
constexpr size_t kJsonCapacity = 4096;
}  // namespace

TranscriptResult ParseDeepgramResponse(const uint8_t* json, size_t length) {
    TranscriptResult result;

    if (json == nullptr || length == 0) {
        return result;
    }

    StaticJsonDocument<kJsonCapacity> doc;
    DeserializationError err = deserializeJson(doc, json, length);
    if (err) {
        system::Logger::warning(kTag, "JSON parse failed: %s", err.c_str());
        return result;
    }

    const char* type = doc["type"] | "";
    if (strcmp(type, "Results") != 0) {
        // Metadata, SpeechStarted, UtteranceEnd, etc. - not a transcript.
        return result;
    }

    JsonObject channel = doc["channel"];
    if (channel.isNull()) {
        return result;
    }

    JsonArray alternatives = channel["alternatives"];
    if (alternatives.isNull() || alternatives.size() == 0) {
        return result;
    }

    JsonObject best = alternatives[0];
    const char* transcript = best["transcript"] | "";

    if (transcript[0] == '\0') {
        // Empty interim result - technically valid, but nothing to show.
        return result;
    }

    result.text = transcript;
    result.confidence = best["confidence"] | 0.0f;
    result.isFinal = doc["is_final"] | false;
    result.speechFinal = doc["speech_final"] | false;
    result.valid = true;
    return result;
}

}  // namespace echolens::transcription
