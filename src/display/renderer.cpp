#include "renderer.h"

#include <algorithm>
#include <sstream>

#include "../system/logger.h"
#include "../../include/constants.h"

namespace echolens::display {

namespace {
constexpr const char* kTag = "Renderer";
}  // namespace

TranscriptRenderer::TranscriptRenderer(OledDisplay& display) : display_(display) {}

void TranscriptRenderer::setText(const std::string& text) {
    fullText_ = text;
    interimText_.clear();
    if (fullText_.size() > constants::kMaxTranscriptChars) {
        // Keep the most recent content; older transcript scrolls out of
        // memory rather than growing unbounded.
        fullText_.erase(0, fullText_.size() - constants::kMaxTranscriptChars);
    }
    rewrap();
    scrollToEnd();
    dirty_ = true;
}

void TranscriptRenderer::setInterimText(const std::string& text) {
    interimText_ = text;
    rewrap();
    scrollToEnd();
    dirty_ = true;
}

void TranscriptRenderer::commitFinalText(const std::string& text, bool speechPaused) {
    if (text.empty()) {
        return;
    }

    std::string finalizedText = text;
    if (speechPaused) {
        appendPauseTerminator(finalizedText);
    }

    std::string candidate = fullText_;
    appendWithSpace(candidate, finalizedText);

    // A logical OLED clear happens only when the current four-line page is
    // full and the next finalized segment would exceed it. Interim text is
    // deliberately excluded: it is a replaceable preview, not page content.
    if (!fullText_.empty() && wrapText(candidate).size() > maxVisibleLines()) {
        fullText_.clear();
        candidate.clear();
        appendWithSpace(candidate, finalizedText);
    }

    fullText_ = candidate;
    if (fullText_.size() > constants::kMaxTranscriptChars) {
        fullText_.erase(0, fullText_.size() - constants::kMaxTranscriptChars);
    }
    interimText_.clear();
    rewrap();
    scrollToEnd();
    dirty_ = true;
}

void TranscriptRenderer::appendText(const std::string& text) {
    commitFinalText(text, false);
}

void TranscriptRenderer::scrollDown() {
    if (wrappedLines_.empty()) {
        return;
    }
    size_t maxOffset = wrappedLines_.size() > maxVisibleLines()
                            ? wrappedLines_.size() - maxVisibleLines()
                            : 0;
    scrollOffset_ = std::min(scrollOffset_ + 1, maxOffset);
}

void TranscriptRenderer::scrollUp() {
    scrollOffset_ = scrollOffset_ > 0 ? scrollOffset_ - 1 : 0;
}

void TranscriptRenderer::scrollToEnd() {
    scrollOffset_ = wrappedLines_.size() > maxVisibleLines()
                        ? wrappedLines_.size() - maxVisibleLines()
                        : 0;
}

void TranscriptRenderer::render() {
    if (!dirty_) {
        return;
    }

    display_.clear();

    size_t visibleLines = maxVisibleLines();
    for (size_t row = 0; row < visibleLines; ++row) {
        size_t lineIndex = scrollOffset_ + row;
        if (lineIndex >= wrappedLines_.size()) {
            break;
        }
        display_.drawLine(static_cast<uint8_t>(row), wrappedLines_[lineIndex].c_str());
    }

    display_.present();
    dirty_ = false;
}

std::vector<std::string> TranscriptRenderer::wrapText(const std::string& text) const {
    std::vector<std::string> lines;
    const size_t maxChars = maxCharsPerLine();
    if (maxChars == 0) {
        return lines;
    }

    std::istringstream words(text);
    std::string word;
    std::string currentLine;

    while (words >> word) {
        // Word-wrap: try to fit whole words; hard-break only if a single
        // word alone exceeds the line width.
        if (word.size() > maxChars) {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
            size_t pos = 0;
            while (pos < word.size()) {
                lines.push_back(word.substr(pos, maxChars));
                pos += maxChars;
            }
            continue;
        }

        size_t prospectiveLen = currentLine.empty() ? word.size() : currentLine.size() + 1 + word.size();
        if (prospectiveLen > maxChars) {
            lines.push_back(currentLine);
            currentLine = word;
        } else {
            if (!currentLine.empty()) {
                currentLine += ' ';
            }
            currentLine += word;
        }
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    if (lines.empty()) {
        lines.push_back("");
    }
    return lines;
}

void TranscriptRenderer::rewrap() {
    std::string visibleText = fullText_;
    appendWithSpace(visibleText, interimText_);
    wrappedLines_ = wrapText(visibleText);
}

void TranscriptRenderer::appendWithSpace(std::string& destination, const std::string& text) {
    if (text.empty()) {
        return;
    }
    if (!destination.empty() && destination.back() != ' ') {
        destination += ' ';
    }
    destination += text;
}

void TranscriptRenderer::appendPauseTerminator(std::string& text) {
    while (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }
    if (text.empty()) {
        return;
    }

    const char last = text.back();
    if (last != '.' && last != '!' && last != '?') {
        text += '.';
    }
}

size_t TranscriptRenderer::maxCharsPerLine() const {
    return display_.widthPx() / OledDisplay::kCharWidthPx;
}

size_t TranscriptRenderer::maxVisibleLines() const {
    const size_t physicalLines = display_.heightPx() / OledDisplay::kCharHeightPx;
    return std::min(physicalLines, constants::kTranscriptDisplayLines);
}

}  // namespace echolens::display
