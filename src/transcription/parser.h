#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// Parses provider transcription JSON payloads into a normalized,
// provider-agnostic result. Uses ArduinoJson exclusively - no manual
// string scanning.

namespace echolens::transcription {

struct TranscriptResult {
    std::string text;
    float confidence = 0.0f;
    bool isFinal = false;
    // True only when the provider's endpointing/VAD detected a pause.
    // isFinal alone can occur mid-utterance.
    bool speechFinal = false;
    bool valid = false;  // false if the payload wasn't a transcript event
};

// Extracts transcript/confidence/is_final from a Deepgram streaming
// response. Returns a TranscriptResult with valid=false if the payload
// is not a "Results" message (e.g. metadata/keepalive frames) or fails
// to parse.
TranscriptResult ParseDeepgramResponse(const uint8_t* json, size_t length);

}  // namespace echolens::transcription
