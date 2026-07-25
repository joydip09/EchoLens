#include "wifi.h"

#include <WiFi.h>

#include "../system/logger.h"
#include "../../include/constants.h"

namespace echolens::network {

namespace {
constexpr const char* kTag = "WiFi";
}  // namespace

WifiManager::WifiManager(const char* ssid, const char* password, uint32_t connectTimeoutMs)
    : ssid_(ssid), password_(password), connectTimeoutMs_(connectTimeoutMs) {}

bool WifiManager::connect() {
    system::Logger::info(kTag, "Connecting to SSID '%s'", ssid_);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);

    const uint32_t startMs = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startMs > connectTimeoutMs_) {
            system::Logger::error(kTag, "Connection timed out");
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    system::Logger::info(kTag, "Connected, IP: %s", WiFi.localIP().toString().c_str());
    lastAttemptMs_ = millis();
    return true;
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WifiManager::poll() {
    if (isConnected()) {
        return;
    }

    const uint32_t now = millis();
    if (now - lastAttemptMs_ < constants::kWifiRetryIntervalMs) {
        return;
    }
    lastAttemptMs_ = now;

    system::Logger::warning(kTag, "Link down, attempting reconnect");
    WiFi.disconnect();
    WiFi.begin(ssid_, password_);
}

}  // namespace echolens::network
