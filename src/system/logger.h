#pragma once

#include <cstdarg>
#include <cstdint>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Centralized logging facility. This is the ONLY module allowed to call
// Serial.print* directly - every other module must route through here so
// log formatting, levels, and (eventually) sinks other than Serial stay
// consistent and swappable in one place.

namespace echolens::system {

enum class LogLevel : uint8_t {
    kDebug = 0,
    kInfo,
    kWarning,
    kError,
};

class Logger {
public:
    // Must be called once, after Serial.begin(), before any log calls.
    static void begin(unsigned long baudRate);

    // Sets the minimum level that will actually be printed. Defaults to Info.
    static void setMinLevel(LogLevel level);

    static void debug(const char* tag, const char* format, ...);
    static void info(const char* tag, const char* format, ...);
    static void warning(const char* tag, const char* format, ...);
    static void error(const char* tag, const char* format, ...);

private:
    static void log(LogLevel level, const char* tag, const char* format, va_list args);
    static const char* levelToString(LogLevel level);

    static LogLevel minLevel_;
    static SemaphoreHandle_t mutex_;
};

}  // namespace echolens::system
