#pragma once

#include <cstddef>
#include <cstdint>
#include <freertos/FreeRTOS.h>

// Compile-time constants shared across modules. No configuration secrets
// live here - see secrets.h for that.

namespace echolens::constants {

// --- Audio capture ---
constexpr uint32_t kSampleRateHz   = 16000;
constexpr uint8_t  kSampleBits     = 16;
constexpr size_t   kI2sDmaBufCount = 8;
constexpr size_t   kI2sDmaBufLen   = 256;  // frames per DMA buffer

// Number of int16_t samples held in the audio ring buffer.
// ~1 second of headroom at 16kHz mono.
constexpr size_t kRingBufferSamples = kSampleRateHz;

// Size of one PCM chunk pulled from the ring buffer and sent upstream.
constexpr size_t kAudioChunkSamples = 800;  // 50 ms @ 16kHz
constexpr size_t kAudioChunkBytes   = kAudioChunkSamples * sizeof(int16_t);

// --- Networking ---
constexpr uint32_t kWifiConnectTimeoutMs    = 15000;
constexpr uint32_t kWifiRetryIntervalMs     = 5000;
constexpr uint32_t kWebSocketReconnectMs    = 3000;
constexpr uint32_t kWebSocketPingIntervalMs = 15000;

// --- Deepgram ---
constexpr const char* kDeepgramHost = "api.deepgram.com";
constexpr const char* kDeepgramPath =
    "/v1/listen?encoding=linear16&sample_rate=16000&channels=1"
    "&interim_results=true&smart_format=true&endpointing=500";
constexpr uint16_t kDeepgramPort = 443;

// --- Display ---
constexpr uint8_t kOledWidth          = 128;
constexpr uint8_t kOledHeight         = 64;
constexpr uint8_t kOledAddress        = 0x3C;
constexpr size_t  kMaxTranscriptChars = 512;
// Keep a readable four-line page. A completed page is replaced only when
// the next finalized text would no longer fit on it.
constexpr size_t  kTranscriptDisplayLines = 4;

// --- FreeRTOS task tuning ---
constexpr uint32_t kAudioTaskStackWords   = 4096;
constexpr uint32_t kNetworkTaskStackWords = 8192;
constexpr uint32_t kDisplayTaskStackWords = 4096;
constexpr uint32_t kSystemTaskStackWords  = 2048;

constexpr UBaseType_t kAudioTaskPriority   = 3;
constexpr UBaseType_t kNetworkTaskPriority = 2;
constexpr UBaseType_t kDisplayTaskPriority = 1;
constexpr UBaseType_t kSystemTaskPriority  = 1;

constexpr size_t kTranscriptQueueLength = 4;

}  // namespace echolens::constants
