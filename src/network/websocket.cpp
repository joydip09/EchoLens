#include "websocket.h"

#include "../system/logger.h"
#include "../../include/constants.h"

namespace echolens::network {

namespace {
constexpr const char* kTag = "WebSocket";
}  // namespace

SecureWebSocketClient::SecureWebSocketClient() = default;

void SecureWebSocketClient::configure(const char* host, uint16_t port, const char* path,
                                       const char* extraHeaders, const char* caCertificate) {
    configured_ = caCertificate != nullptr && caCertificate[0] != '\0';
    if (!configured_) {
        system::Logger::error(kTag, "No CA certificate configured; refusing insecure TLS");
        return;
    }

    client_.beginSslWithCA(host, port, path, caCertificate);
    if (extraHeaders != nullptr) {
        client_.setExtraHeaders(extraHeaders);
    }
    client_.setReconnectInterval(constants::kWebSocketReconnectMs);
    client_.enableHeartbeat(constants::kWebSocketPingIntervalMs,
                             constants::kWebSocketPingIntervalMs / 2, 2);

    client_.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
        handleEvent(type, payload, length);
    });
}

void SecureWebSocketClient::onMessage(system::WebSocketMessageCallback callback) {
    messageCallback_ = std::move(callback);
}

void SecureWebSocketClient::onConnectionChange(system::WebSocketConnectionCallback callback) {
    connectionCallback_ = std::move(callback);
}

void SecureWebSocketClient::begin() {
    if (configured_) {
        // The client is configured above; loop() drives it non-blockingly.
        system::Logger::info(kTag, "WebSocket client starting");
    }
}

void SecureWebSocketClient::poll() {
    if (configured_) {
        client_.loop();
    }
}

bool SecureWebSocketClient::sendBinary(const uint8_t* data, size_t length) {
    if (!connected_) {
        return false;
    }
    return client_.sendBIN(data, length);
}

bool SecureWebSocketClient::sendText(const char* text) {
    if (!connected_) {
        return false;
    }
    return client_.sendTXT(text);
}

void SecureWebSocketClient::handleEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_CONNECTED:
            connected_ = true;
            system::Logger::info(kTag, "Connected");
            if (connectionCallback_) connectionCallback_(true);
            break;

        case WStype_DISCONNECTED:
            connected_ = false;
            system::Logger::warning(kTag, "Disconnected");
            if (connectionCallback_) connectionCallback_(false);
            break;

        case WStype_TEXT:
            if (messageCallback_) messageCallback_(payload, length, false);
            break;

        case WStype_BIN:
            if (messageCallback_) messageCallback_(payload, length, true);
            break;

        case WStype_ERROR:
            system::Logger::error(kTag, "Transport error");
            break;

        default:
            break;
    }
}

}  // namespace echolens::network
