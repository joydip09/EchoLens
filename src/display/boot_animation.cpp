#include "boot_animation.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace echolens::display {

namespace {
constexpr TickType_t kFrameDuration = pdMS_TO_TICKS(180);
constexpr const char* kFrames[] = {"[>   ]", "[>>  ]", "[>>> ]", "[>>>>]"};
constexpr size_t kFrameCount = sizeof(kFrames) / sizeof(kFrames[0]);
}  // namespace

BootAnimation::BootAnimation(OledDisplay& display) : display_(display) {}

void BootAnimation::play() {
    for (size_t index = 0; index < kFrameCount; ++index) {
        display_.clear();
        display_.drawLine(1, "   EchoLens");
        display_.drawLine(4, kFrames[index]);
        display_.present();
        vTaskDelay(kFrameDuration);
    }
}

}  // namespace echolens::display
