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

void WifiManager::begin() {
    system::Logger::info(kTag, "Connecting to SSID '%s'", ssid_);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid_, password_);
    lastAttemptMs_ = millis();
    started_ = true;
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WifiManager::poll() {
    if (isConnected()) {
        return;
    }

    if (!started_) {
        begin();
        return;
    }

    const uint32_t now = millis();
    if (now - lastAttemptMs_ < constants::kWifiRetryIntervalMs) {
        return;
    }
    lastAttemptMs_ = now;

    system::Logger::warning(kTag, "Link down, attempting reconnect");
    WiFi.disconnect();
    begin();
}

}  // namespace echolens::network
