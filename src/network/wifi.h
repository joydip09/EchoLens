#pragma once

#include <cstdint>

// Owns the WiFi station connection lifecycle: connect, monitor, and
// reconnect. Knows nothing about the microphone, the OLED, WebSockets,
// or Deepgram - it exposes only connectivity state.

namespace echolens::network {

class WifiManager {
public:
    WifiManager(const char* ssid, const char* password, uint32_t connectTimeoutMs);

    // Blocks (bounded by connectTimeoutMs) until associated or timed out.
    // Returns true on success.
    bool connect();

    bool isConnected() const;

    // Non-blocking: call periodically from the network task. Re-initiates
    // a connection attempt if the link has dropped and enough time has
    // passed since the last attempt.
    void poll();

private:
    const char* ssid_;
    const char* password_;
    uint32_t connectTimeoutMs_;
    uint32_t lastAttemptMs_ = 0;
};

}  // namespace echolens::network
