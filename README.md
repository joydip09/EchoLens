# EchoLens

> A real-time wearable speech transcription system powered by the ESP32-S3.

---

# Overview

EchoLens is an embedded systems project that captures live speech using an I2S microphone, streams audio to a cloud speech recognition service over a secure WebSocket connection, and displays the resulting transcript on a small OLED display with minimal latency.

The project is designed as an engineering exercise in building a modular, maintainable, and extensible embedded application rather than simply creating a working prototype.

The long-term objective is to build a wearable assistive device capable of displaying live captions in real time while remaining flexible enough to support future features such as translation, offline transcription, speaker identification, and AI-assisted interaction.

---

# Project Goals

The project emphasizes software architecture as much as functionality.

Primary goals include:

- Real-time audio capture
- Low-latency speech transcription
- Smooth OLED rendering
- Stable Wi-Fi communication
- Modular software architecture
- Clean separation of responsibilities
- High maintainability
- Future extensibility

The codebase should remain understandable even as additional features are added.

---

# Current Hardware

## Microcontroller

ESP32-S3-WROOM-1-N16R8

- Dual-core Xtensa LX7
- 16 MB Flash
- 8 MB PSRAM
- Native USB
- Wi-Fi
- Bluetooth (currently unused)

---

## Microphone

INMP441

Interface:

- I2S

Responsibilities:

- Capture raw PCM audio
- Continuous streaming
- Mono input

---

## Display

SSD1306 OLED

Resolution:

128 × 64 pixels

Interface:

- I²C

Responsibilities:

- Display transcript
- Word wrapping
- Automatic scrolling
- Status indicators

---

# Software Stack

## Framework

Arduino Framework

---

## Build System

PlatformIO

The committed PlatformIO environment targets the ESP32-S3-WROOM-1-N16R8
(16 MB flash, 8 MB OPI PSRAM). Confirm the physical module and wiring before
uploading.

---

## Language

C++17

---

## Communication

- Wi-Fi
- TLS
- Secure WebSocket

---

## Speech Recognition

Current provider:

Deepgram Live API

The software should remain provider-independent so other services can be integrated later without changing unrelated modules.

---

# Design Philosophy

EchoLens is intentionally organized as a collection of independent modules.

Every module should have a single responsibility.

Modules should communicate through well-defined interfaces rather than directly depending on each other.

Whenever possible:

- High cohesion
- Low coupling
- Explicit ownership
- Clear data flow

The objective is to make replacing one subsystem require little or no modification to others.

---

# System Architecture

The application follows a streaming pipeline.

```
Microphone
      │
      ▼
Audio Capture
      │
      ▼
Ring Buffer
      │
      ▼
Network Streaming
      │
      ▼
Speech Recognition
      │
      ▼
Transcript Parser
      │
      ▼
Display Renderer
      │
      ▼
OLED
```

Each stage performs exactly one task.

---

# Planned Project Structure

```
EchoLens/
│
├── include/
│
├── src/
│   ├── audio/
│   ├── network/
│   ├── transcription/
│   ├── display/
│   └── system/
│
├── lib/
├── docs/
├── test/
│
├── platformio.ini
├── README.md
├── AGENTS.md
└── LICENSE
```

---

# Module Responsibilities

## audio/

Responsible only for microphone capture.

Responsibilities:

- Initialize I2S
- Read microphone samples
- Normalize audio
- Deliver PCM frames

This module must never know how networking works.

---

## network/

Responsible only for communication.

Responsibilities:

- Wi-Fi connection
- Reconnection
- TLS
- WebSocket
- Packet transmission

This module should never manipulate audio or display data.

---

## transcription/

Responsible for speech recognition.

Responsibilities:

- Communicate with transcription provider
- Receive responses
- Parse JSON
- Produce transcript strings

No display logic belongs here.

---

## display/

Responsible for rendering.

Responsibilities:

- Word wrapping
- Line buffering
- Scrolling
- Cursor management
- OLED updates

Display code should not know where transcripts originate.

---

## system/

Shared infrastructure.

Examples include:

- Ring buffer
- Logger
- Timing utilities
- Future queues
- Common helpers

---

# Runtime Architecture

The project is intended to use FreeRTOS tasks instead of placing all logic inside `loop()`.

Planned tasks include:

## Audio Task

Captures microphone data continuously.

Produces PCM frames.

---

## Network Task

Consumes audio frames.

Maintains WebSocket connection.

Streams audio.

Receives transcription responses.

---

## Display Task

Consumes parsed transcript messages.

Updates OLED efficiently.

---

Future tasks may include:

- Battery monitoring
- User interface
- BLE
- Translation
- AI assistant

---

# Data Flow

```
INMP441

↓

PCM Samples

↓

Ring Buffer

↓

WebSocket

↓

Deepgram

↓

JSON Response

↓

Transcript

↓

Renderer

↓

OLED
```

---

# Memory Strategy

The project aims to minimize unnecessary allocations during runtime.

General principles:

- Prefer static allocation where practical.
- Reuse buffers whenever possible.
- Avoid frequent heap allocations.
- Keep the audio path allocation-free after initialization.
- Use PSRAM only where appropriate.

These guidelines may evolve as implementation progresses.

---

# Error Handling

The application should continue operating whenever recovery is possible.

Expected recovery mechanisms include:

- Wi-Fi reconnection
- WebSocket reconnection
- Provider reconnect
- Audio buffer recovery
- Timeout handling

Errors should be logged and isolated rather than causing a complete restart whenever feasible.

---

# Logging

Logging should assist development without cluttering application logic.

Typical log categories:

- System
- Audio
- Network
- Transcription
- Display

Logging behavior may later become configurable.

---

# Coding Principles

The project favors readable code over clever code.

General principles:

- Small functions
- Clear interfaces
- Descriptive names
- Minimal global state
- Explicit ownership
- Consistent formatting

Maintainability is considered more important than writing the shortest possible code.

---

# Development Roadmap

## Phase 0

Project initialization.

- PlatformIO
- Build verification
- Repository setup

---

## Phase 1

Hardware verification.

- Wi-Fi
- OLED
- Microphone

---

## Phase 2

Audio capture.

- I2S initialization
- PCM acquisition
- Audio validation

---

## Phase 3

Networking.

- Secure WebSocket
- Provider connection

---

## Phase 4

Audio streaming.

- Ring buffer
- Continuous transmission

---

## Phase 5

Transcription.

- JSON parsing
- Transcript extraction

---

## Phase 6

Display.

- Text rendering
- Scrolling
- Word wrapping

---

## Phase 7

Optimization.

- FreeRTOS tasks
- Buffer tuning
- Latency reduction

---

## Phase 8

Robustness.

- Recovery logic
- Error handling
- Stability improvements

---

# Future Features

The architecture is intentionally designed to support future expansion.

Potential additions include:

- Local Whisper transcription
- Multiple speech providers
- Automatic language detection
- Live translation
- Speaker recognition
- Voice activity detection
- Battery management
- Configuration interface
- OTA firmware updates
- BLE companion application
- AI assistant integration

Not all planned features are guaranteed to be implemented.

---

# Development Status

The project is currently in Reviewing and Debugging Phase.

---

# First-Time Setup

1. Copy `include/secrets.example.h` to `include/secrets.h`.
2. Fill in the Wi-Fi and Deepgram values in `secrets.h`.
3. Build with `pio run`.

`secrets.h` is intentionally ignored by Git. Do not commit it.

The public ISRG Root X1 trust anchor required by Deepgram is committed in
`include/certificates.h`; review it when updating dependencies or if the
provider's certificate chain changes.
