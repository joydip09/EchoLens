#pragma once

#include <cstdint>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Thin hardware wrapper around the SSD1306 driver. Owns I2C init and raw
// draw primitives only - no word wrapping, no scrolling, no knowledge of
// where text comes from. That logic lives in TranscriptRenderer.

namespace echolens::display {

struct OledConfig {
    uint8_t width;
    uint8_t height;
    uint8_t i2cAddress;
    int sdaPin;
    int sclPin;
};

class OledDisplay {
public:
    explicit OledDisplay(const OledConfig& config);

    bool begin();

    void clear();
    void drawLine(uint8_t lineIndex, const char* text);
    void present();

    uint8_t widthPx() const { return config_.width; }
    uint8_t heightPx() const { return config_.height; }

    // Font is 6x8px at text size 1, exposed so the renderer can compute
    // wrapping without duplicating a magic number.
    static constexpr uint8_t kCharWidthPx  = 6;
    static constexpr uint8_t kCharHeightPx = 8;

private:
    OledConfig config_;
    Adafruit_SSD1306 driver_;
};

}  // namespace echolens::display
