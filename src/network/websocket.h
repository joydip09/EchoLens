#pragma once

#include <cstddef>
#include <cstdint>
#include <WebSocketsClient.h>

#include "../system/websocket_transport.h"

// Thin, provider-agnostic secure WebSocket transport. Owns connection,
// TLS, reconnect, and framing concerns only. It does not know it is
// talking to Deepgram specifically - callers supply a host/path/headers
// and a message callback, so this class is reusable for any future
// WebSocket-based provider.

namespace echolens::network {

class SecureWebSocketClient : public system::WebSocketTransport {
public:
    SecureWebSocketClient();

    // Extra headers (e.g. "Authorization: Token ...") appended verbatim.
    void configure(const char* host, uint16_t port, const char* path,
                   const char* extraHeaders, const char* caCertificate) override;

    void onMessage(system::WebSocketMessageCallback callback) override;
    void onConnectionChange(system::WebSocketConnectionCallback callback) override;

    void begin() override;

    // Must be pumped frequently (e.g. every loop iteration of the owning
    // task) - the underlying library is callback-driven but non-blocking.
    void poll() override;

    bool isConnected() const override { return connected_; }

    bool sendBinary(const uint8_t* data, size_t length) override;
    bool sendText(const char* text) override;

private:
    void handleEvent(WStype_t type, uint8_t* payload, size_t length);

    WebSocketsClient client_;
    system::WebSocketMessageCallback messageCallback_;
    system::WebSocketConnectionCallback connectionCallback_;
    bool connected_ = false;
    bool configured_ = false;
};

}  // namespace echolens::network
