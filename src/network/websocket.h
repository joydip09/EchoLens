#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

#include <WebSocketsClient.h>

// Thin, provider-agnostic secure WebSocket transport. Owns connection,
// TLS, reconnect, and framing concerns only. It does not know it is
// talking to Deepgram specifically - callers supply a host/path/headers
// and a message callback, so this class is reusable for any future
// WebSocket-based provider.

namespace echolens::network {

using MessageCallback = std::function<void(const uint8_t* data, size_t length, bool isBinary)>;
using ConnectionCallback = std::function<void(bool connected)>;

class SecureWebSocketClient {
public:
    SecureWebSocketClient();

    // Extra headers (e.g. "Authorization: Token ...") appended verbatim.
    void configure(const char* host, uint16_t port, const char* path, const char* extraHeaders);

    void onMessage(MessageCallback callback);
    void onConnectionChange(ConnectionCallback callback);

    void begin();

    // Must be pumped frequently (e.g. every loop iteration of the owning
    // task) - the underlying library is callback-driven but non-blocking.
    void poll();

    bool isConnected() const { return connected_; }

    bool sendBinary(const uint8_t* data, size_t length);
    bool sendText(const char* text);

private:
    void handleEvent(WStype_t type, uint8_t* payload, size_t length);

    WebSocketsClient client_;
    MessageCallback messageCallback_;
    ConnectionCallback connectionCallback_;
    bool connected_ = false;
};

}  // namespace echolens::network
