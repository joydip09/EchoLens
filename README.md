# EchoLens

> Real-time speech transcription on an ESP32-S3 using Deepgram Streaming API.

EchoLens is an embedded speech transcription system built around the ESP32-S3. It captures audio from an I2S microphone, streams it to the Deepgram Streaming API over a secure WebSocket connection, receives live transcripts, and displays them on a 128×64 OLED display.

The project is designed with modularity and scalability in mind, allowing future expansion into multilingual transcription, translation, speaker identification, song lyric transcription, transcript logging, and web-based monitoring.

---

## Features

### Current Goals (v1)

- Live speech transcription
- ESP32-S3 based
- INMP441 I2S microphone
- Deepgram Streaming API
- Secure WebSocket communication
- SSD1306 OLED output
- Modular firmware architecture
- FreeRTOS-based task separation

---

## Planned Features

- Multilingual transcription
- Translation mode
- Song lyric transcription
- Transcript logging
- SD card support
- Web dashboard
- Mobile companion application
- Speaker identification
- AI summarization
- Offline transcription support

---

# Hardware

| Component          | Model                  |
| ------------------ | ---------------------- |
| MCU                | ESP32-S3-WROOM-1-N16R8 |
| Microphone         | INMP441                |
| Display            | SSD1306 OLED 128×64    |
| Communication      | Wi-Fi                  |
| Speech Recognition | Deepgram Streaming API |

---

# Architecture

```
        INMP441
           │
           ▼
    Audio Capture Task
           │
           ▼
     Audio Ring Buffer
           │
           ▼
   Network/WebSocket Task
           │
           ▼
      Deepgram API
           │
           ▼
     JSON Transcript
           │
           ▼
    Transcript Parser
           │
           ▼
      Display Task
           │
           ▼
      SSD1306 OLED
```

---

# Project Structure

```
EchoLens
│
├── platformio.ini
├── README.md
├── .gitignore
│
├── include/
│   ├── config.h
│   ├── constants.h
│   ├── pins.h
│   ├── secrets.h
│   └── version.h
│
├── src/
│   ├── main.cpp
│   │
│   ├── audio/
│   ├── display/
│   ├── network/
│   ├── system/
│   └── transcription/
│
├── lib/
├── test/
└── docs/
```

---

# Software Stack

- PlatformIO
- Arduino Framework
- ESP32 Arduino Core
- FreeRTOS
- ArduinoJson
- WebSockets
- Adafruit SSD1306
- Adafruit GFX

---

# Repository Philosophy

EchoLens follows a modular architecture.

Each module has a single responsibility.

| Module        | Responsibility                          |
| ------------- | --------------------------------------- |
| audio         | Capture microphone samples              |
| network       | Wi-Fi and WebSocket communication       |
| transcription | Deepgram communication and JSON parsing |
| display       | OLED rendering                          |
| system        | Shared utilities and infrastructure     |

This separation makes the project easier to maintain and allows individual components to be replaced without affecting the rest of the system.

---

# Development Roadmap

## Phase 0

- Project initialization
- Verify toolchain
- Hello World

---

## Phase 1

Hardware verification

- Wi-Fi
- OLED
- INMP441

---

## Phase 2

Audio subsystem

- Configure I2S
- Capture PCM
- Verify microphone quality

---

## Phase 3

Networking

- Wi-Fi
- TLS
- Secure WebSocket
- Deepgram connection

---

## Phase 4

Streaming

- Continuous audio streaming
- Ring buffer
- Packet scheduling

---

## Phase 5

Transcription

- Receive JSON
- Parse transcript
- Print to Serial

---

## Phase 6

Display

- OLED rendering
- Word wrapping
- Scrolling text

---

## Phase 7

Optimization

- FreeRTOS tasks
- Queue optimization
- Memory optimization
- Latency reduction

---

## Phase 8

Reliability

- Automatic reconnect
- Error recovery
- Watchdog
- Buffer protection

---

# FreeRTOS Task Layout

```
Audio Task
    │
    ▼
Audio Ring Buffer
    │
    ▼
Network Task
    │
    ▼
Transcript Queue
    │
    ▼
Display Task
```

Each task performs one well-defined job and communicates using queues or buffers instead of directly calling one another.

---

# Getting Started

## Clone

```bash
git clone https://github.com/<username>/EchoLens.git

cd EchoLens
```

---

## Install Dependencies

Open the project using PlatformIO in VS Code.

PlatformIO will automatically install all required libraries defined in `platformio.ini`.

---

## Configure Secrets

Create:

```
include/secrets.h
```

Example:

```cpp
#pragma once

#define WIFI_SSID "YOUR_WIFI"
#define WIFI_PASSWORD "YOUR_PASSWORD"

#define DEEPGRAM_API_KEY "YOUR_API_KEY"
```

This file is ignored by Git.

---

## Build

```
PlatformIO: Build
```

or

```bash
pio run
```

---

## Upload

```bash
pio run --target upload
```

---

## Serial Monitor

```bash
pio device monitor
```

---

# Dependencies

- ArduinoJson
- WebSockets
- Adafruit SSD1306
- Adafruit GFX

All dependencies are managed automatically by PlatformIO.

---

# Coding Guidelines

- One responsibility per module.
- Keep functions short and focused.
- Prefer `constexpr` over macros where appropriate.
- Avoid global variables unless necessary.
- Separate hardware, networking, and application logic.
- Commit small, working changes frequently.

---

# Git Workflow

Example milestones:

```
Initial project

↓

OLED initialized

↓

Microphone initialized

↓

Wi-Fi connected

↓

Deepgram connected

↓

Streaming audio

↓

Receiving transcripts

↓

OLED rendering

↓

Version 1.0
```

---

# Future Enhancements

- Translation
- Multiple transcription providers
- Local Whisper server support
- Web interface
- Transcript storage
- Speaker diarization
- AI summarization
- OTA firmware updates
- SD card logging
- Bluetooth configuration

---

# License

This project is licensed under the MIT License.

---

# Acknowledgements

- Espressif Systems
- Deepgram
- PlatformIO
- Arduino
- Adafruit
