#include "oled.h"

#include <Wire.h>

#include "../system/logger.h"

namespace echolens::display {

namespace {
constexpr const char* kTag = "OLED";
constexpr int kNoResetPin = -1;
}  // namespace

OledDisplay::OledDisplay(const OledConfig& config)
    : config_(config), driver_(config.width, config.height, &Wire, kNoResetPin) {}

bool OledDisplay::begin() {
    Wire.begin(config_.sdaPin, config_.sclPin);

    if (!driver_.begin(SSD1306_SWITCHCAPVCC, config_.i2cAddress)) {
        system::Logger::error(kTag, "SSD1306 init failed at 0x%02X", config_.i2cAddress);
        return false;
    }

    driver_.setTextColor(SSD1306_WHITE);
    driver_.setTextSize(1);
    driver_.cp437(true);
    clear();
    present();

    system::Logger::info(kTag, "Display initialized (%ux%u)", config_.width, config_.height);
    return true;
}

void OledDisplay::clear() {
    driver_.clearDisplay();
}

void OledDisplay::drawLine(uint8_t lineIndex, const char* text) {
    if (text == nullptr) {
        return;
    }
    driver_.setCursor(0, lineIndex * kCharHeightPx);
    driver_.print(text);
}

void OledDisplay::present() {
    driver_.display();
}

}  // namespace echolens::display
