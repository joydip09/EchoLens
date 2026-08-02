#pragma once

#include <cstdint>

#include "constants.h"
#include "certificates.h"
#include "pins.h"
#include "secrets.h"

// Aggregated runtime configuration. Modules depend on this struct rather
// than reaching into secrets.h / constants.h directly wherever practical -
// this keeps future dynamic provisioning (captive portal, NVS-backed
// settings, etc.) a drop-in change instead of a rewrite.

namespace echolens {

struct WifiConfig {
  const char *ssid = secrets::kWifiSsid;
  const char *password = secrets::kWifiPassword;
  uint32_t connectTimeoutMs = constants::kWifiConnectTimeoutMs;
};

struct DeepgramConfig {
  const char *apiKey = secrets::kDeepgramApiKey;
  const char *host = constants::kDeepgramHost;
  const char *path = constants::kDeepgramPath;
  uint16_t port = constants::kDeepgramPort;
  const char *caCertificate = certificates::kDeepgramRootCa;
};

struct AppConfig {
  WifiConfig wifi;
  DeepgramConfig deepgram;
};

} // namespace echolens
