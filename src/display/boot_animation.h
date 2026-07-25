#pragma once

#include "oled.h"

namespace echolens::display {

// A short startup animation that confirms the OLED is responsive before the
// asynchronous audio, network, and display tasks begin.
class BootAnimation {
public:
    explicit BootAnimation(OledDisplay& display);

    void play();

private:
    OledDisplay& display_;
};

}  // namespace echolens::display
