#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>

namespace echolens::system {

using WebSocketMessageCallback = std::function<void(const uint8_t* data, size_t length, bool isBinary)>;
using WebSocketConnectionCallback = std::function<void(bool connected)>;

// Provider-neutral interface shared by transcription providers and network
// implementations. It prevents the transcription module from depending on a
// concrete network implementation.
class WebSocketTransport {
public:
    virtual ~WebSocketTransport() = default;

    virtual void configure(const char* host, uint16_t port, const char* path,
                           const char* extraHeaders, const char* caCertificate) = 0;
    virtual void onMessage(WebSocketMessageCallback callback) = 0;
    virtual void onConnectionChange(WebSocketConnectionCallback callback) = 0;
    virtual void begin() = 0;
    virtual void poll() = 0;
    virtual bool isConnected() const = 0;
    virtual bool sendBinary(const uint8_t* data, size_t length) = 0;
    virtual bool sendText(const char* text) = 0;
};

}  // namespace echolens::system
