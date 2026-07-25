#include "logger.h"

#include <Arduino.h>
#include <cstdio>

namespace echolens::system {

LogLevel Logger::minLevel_ = LogLevel::kInfo;
SemaphoreHandle_t Logger::mutex_ = nullptr;

void Logger::begin(unsigned long baudRate) {
    Serial.begin(baudRate);
    if (mutex_ == nullptr) {
        mutex_ = xSemaphoreCreateMutex();
    }
}

void Logger::setMinLevel(LogLevel level) {
    minLevel_ = level;
}

void Logger::debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::kDebug, tag, format, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::kInfo, tag, format, args);
    va_end(args);
}

void Logger::warning(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::kWarning, tag, format, args);
    va_end(args);
}

void Logger::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::kError, tag, format, args);
    va_end(args);
}

void Logger::log(LogLevel level, const char* tag, const char* format, va_list args) {
    if (level < minLevel_ || mutex_ == nullptr) {
        return;
    }

    char message[192];
    vsnprintf(message, sizeof(message), format, args);

    if (xSemaphoreTake(mutex_, pdMS_TO_TICKS(50)) == pdTRUE) {
        Serial.printf("[%8lu][%s][%s] %s\n", millis(), levelToString(level), tag, message);
        xSemaphoreGive(mutex_);
    }
}

const char* Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug:   return "DEBUG";
        case LogLevel::kInfo:    return "INFO ";
        case LogLevel::kWarning: return "WARN ";
        case LogLevel::kError:   return "ERROR";
    }
    return "?????";
}

}  // namespace echolens::system
