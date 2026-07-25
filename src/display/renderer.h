#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "oled.h"

// Turns arbitrary transcript text into wrapped, scrollable lines on the
// OLED. Deliberately unaware of where the text originates (Deepgram,
// future providers, or a test harness) - it only knows about strings and
// pixels.

namespace echolens::display {

class TranscriptRenderer {
public:
    explicit TranscriptRenderer(OledDisplay& display);

    // Replaces the current display page and re-wraps it for the display.
    void setText(const std::string& text);

    // Shows a non-final provider hypothesis without committing it to the
    // display page. The next hypothesis replaces it.
    void setInterimText(const std::string& text);

    // Commits one finalized provider segment. When it would overflow the
    // display page, the page is cleared before the new segment is added.
    // A sentence terminator is appended only for a detected speech pause.
    void commitFinalText(const std::string& text, bool speechPaused);

    // Appends text directly to the committed display page.
    void appendText(const std::string& text);

    // Advances the scroll window by one line. No-op if already at the end.
    void scrollDown();
    void scrollUp();
    void scrollToEnd();

    // Draws the current scroll window to the display and pushes the
    // framebuffer out. Call once per display task tick.
    void render();

private:
    static void appendWithSpace(std::string& destination, const std::string& text);
    static void appendPauseTerminator(std::string& text);
    std::vector<std::string> wrapText(const std::string& text) const;
    void rewrap();
    size_t maxCharsPerLine() const;
    size_t maxVisibleLines() const;

    OledDisplay& display_;
    std::string fullText_;
    std::string interimText_;
    std::vector<std::string> wrappedLines_;
    size_t scrollOffset_ = 0;
    bool dirty_ = true;
};

}  // namespace echolens::display
