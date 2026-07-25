#pragma once

// Physical pin mapping for the ESP32-S3-WROOM-1-N16R8 target.
// Verify against actual wiring before flashing - these are project defaults.

namespace echolens::pins {

// --- INMP441 I2S microphone ---
constexpr int kI2sBclk = 5; // SCK / bit clock
constexpr int kI2sWs = 4;   // WS  / word select (L/R clock)
constexpr int kI2sDin = 6;  // SD  / serial data out from mic

// --- SSD1306 OLED (I2C) ---
// Defaults kept consistent with other ESP32-S3 boards used on this project;
// confirm against actual wiring before flashing.
constexpr int kI2cSda = 8;
constexpr int kI2cScl = 9;

} // namespace echolens::pins
