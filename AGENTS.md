# AGENTS.md

> **Purpose**
>
> This document defines the engineering principles, coding standards, architectural constraints, and development workflow for the EchoLens project.
>
> Every AI coding agent (Codex, ChatGPT, Claude Code, Gemini CLI, Cursor, Copilot, etc.) must treat this document as the authoritative engineering guide.
>
> The objective is **not merely to produce working code**, but to preserve the project's architecture, maintainability, and long-term scalability.

---

# 1. Project Overview

EchoLens is a real-time speech transcription system built around an ESP32-S3.

Pipeline:

```
INMP441
    ↓
Audio Capture
    ↓
Ring Buffer
    ↓
Secure WebSocket
    ↓
Deepgram Live API
    ↓
JSON Parser
    ↓
Renderer
    ↓
SSD1306 OLED
```

The software is intentionally modular.

Every module should perform exactly one responsibility.

---

# 2. Engineering Philosophy

The project prioritizes:

- correctness
- readability
- maintainability
- extensibility
- determinism
- modularity

Performance matters, but never at the expense of architecture.

Whenever there is a conflict:

```
Correctness
    >
Maintainability
    >
Readability
    >
Performance
```

Micro-optimizations are discouraged unless profiling demonstrates a real need.

---

# 3. General Rules

Always:

- understand the existing architecture before modifying code
- preserve module boundaries
- keep responsibilities separated
- prefer composition over tightly coupled code
- keep interfaces explicit
- preserve backward compatibility unless instructed otherwise

Never:

- rewrite unrelated modules
- refactor large portions of the project without permission
- introduce hidden behavior
- silently change APIs
- add unnecessary dependencies

---

# 4. Coding Style

Language:

- C++17

Formatting:

- consistent indentation
- descriptive identifiers
- avoid abbreviations unless universally understood
- use namespaces
- one class per responsibility

Prefer:

```
constexpr
enum class
std::array
std::span (when available)
```

Avoid:

```
#define

magic numbers

anonymous globals
```

---

# 5. Naming Convention

Classes

```
Renderer
DeepgramClient
AudioBuffer
```

Methods

```
initialize()

connect()

render()

readSamples()
```

Variables

```
sampleRate
frameCount
wifiConnected
```

Constants

```
kSampleRateHz
kFrameDurationMs
kMaxPacketBytes
```

Private members

```
mSocket
mDisplay
mRingBuffer
```

---

# 6. Project Structure

```
include/

src/

lib/

docs/

test/
```

Each folder has a clear responsibility.

Do not move files between folders unless necessary.

---

# 7. Module Responsibilities

## audio/

Owns:

- I2S
- microphone
- PCM generation

Must NOT:

- use Wi-Fi
- parse JSON
- update OLED

---

## network/

Owns:

- Wi-Fi
- TLS
- WebSocket

Must NOT:

- process audio
- draw graphics

---

## transcription/

Owns:

- provider communication
- JSON parsing
- transcript extraction

Must NOT:

- access hardware directly

---

## display/

Owns:

- rendering
- scrolling
- word wrapping
- cursor management

Must NOT:

- communicate with network

---

## system/

Owns:

- logger
- utilities
- queues
- ring buffer
- shared helpers

---

# 8. Dependency Direction

Allowed:

```
display
    ↓
system

network
    ↓
system

audio
    ↓
system

transcription
    ↓
system
```

Avoid direct dependencies between sibling modules. When `transcription/`
needs transport behavior, it may depend only on a provider-neutral interface
in `system/`; `network/` supplies the concrete implementation and `main.cpp`
injects it. This keeps provider code independent of network implementations.

Example:

```
audio

↓

network

↓

display
```

is NOT acceptable.

Communication should happen through well-defined interfaces.

---

# 9. FreeRTOS Architecture

The project is task-oriented.

Preferred tasks:

```
Audio Task

Network Task

Display Task
```

Future tasks:

```
Battery

UI

BLE

Translation
```

Avoid large monolithic tasks.

Each task should own exactly one responsibility.

---

# 10. Communication Between Tasks

Preferred mechanisms:

- Queue
- Ring Buffer
- Event Group
- Semaphore

Avoid:

- global flags
- shared mutable state
- busy waiting

---

# 11. Memory Rules

The audio path is performance-critical.

After initialization:

Avoid:

- malloc
- new
- delete
- realloc

Prefer:

- static buffers
- reusable buffers
- stack allocation where practical

Heap allocations are acceptable during startup.

---

# 12. Real-Time Constraints

Never block the audio task.

Avoid:

```
delay()

delayMicroseconds()

lengthy Serial output

network operations

OLED drawing
```

inside audio capture.

The audio pipeline has the highest priority.

---

# 13. Error Handling

Recover whenever possible.

Recoverable examples:

- Wi-Fi disconnect
- WebSocket disconnect
- provider timeout

Do not reboot immediately.

Attempt graceful recovery first.

---

# 14. Logging

Logging is encouraged.

Avoid excessive logging inside timing-critical code.

Preferred categories:

```
SYSTEM

AUDIO

NETWORK

TRANSCRIPTION

DISPLAY
```

Logging should help debugging without overwhelming the serial monitor.

---

# 15. Configuration

Configuration belongs inside:

```
config.h
```

Secrets belong inside:

```
secrets.h
```

Never hardcode:

- Wi-Fi SSID
- Wi-Fi password
- API keys

Never commit secrets.

---

# 16. Constants

Shared compile-time values belong in:

```
constants.h
```

Avoid duplicate values throughout the project.

---

# 17. Hardware Assumptions

Current hardware:

- ESP32-S3-WROOM-1-N16R8
- INMP441
- SSD1306 OLED

Future hardware support should remain possible.

Do not unnecessarily hardcode board-specific assumptions.

---

# 18. Networking

Maintain a persistent secure WebSocket.

Reconnect automatically.

Handle:

- disconnects
- timeouts
- invalid packets

Do not block the application waiting for the network.

---

# 19. Audio

Responsibilities:

- initialize I2S
- capture PCM
- normalize samples

Never:

- compress
- interpret speech
- display data

Audio should remain provider-independent.

---

# 20. Transcription Provider

Current provider:

Deepgram Live API

Future providers may include:

- Whisper
- AssemblyAI
- Groq
- custom local models

Design the interface so providers can be swapped without changing unrelated modules.

---

# 21. Display

The display module receives text.

It should never know:

- where text originated
- which provider produced it

Responsibilities:

- wrapping
- scrolling
- rendering

Nothing more.

---

# 22. JSON

JSON parsing belongs exclusively inside:

```
transcription/
```

Never parse provider responses inside networking code.

---

# 23. Performance

Optimize only after profiling.

Do not sacrifice readability for speculative performance gains.

Expected optimization priorities:

1. latency

2. memory

3. CPU usage

---

# 24. Code Changes

When modifying code:

Read related files first.

Understand current behavior.

Avoid speculative refactoring.

Keep edits minimal.

Preserve existing interfaces whenever practical.

---

# 25. New Features

Before implementing:

Determine:

- which module owns it
- whether a new abstraction is required
- whether interfaces should change

Never insert unrelated logic into an existing module simply because it compiles.

---

# 26. Git Workflow

Small commits.

Single-purpose commits.

Examples:

```
Initialize project

Add OLED driver

Implement ring buffer

Connect Deepgram

Add transcript parser

Improve reconnection
```

Avoid "misc fixes" commits.

---

# 27. Pull Requests

Every logical change should include:

- reason
- affected modules
- summary
- testing notes

---

# 28. Testing

Every major feature should be testable independently.

Examples:

Audio:

Can capture PCM.

Network:

Can connect independently.

Display:

Can render mock text.

Transcription:

Can parse stored JSON.

Modules should not require the entire system for verification whenever avoidable.

---

# 29. Documentation

When architecture changes:

Update:

- README.md
- AGENTS.md
- docs/

Documentation is part of the project, not an afterthought.

---

# 30. Things to Avoid

Do not:

- create giant source files
- place everything in main.cpp
- duplicate utilities
- introduce circular dependencies
- hide complex behavior
- silently ignore errors
- use globals as message passing
- block FreeRTOS tasks unnecessarily
- optimize prematurely

---

# 31. Before Writing Code

Always ask:

1.

Who owns this functionality?

2.

Does it belong in an existing module?

3.

Should a new abstraction exist?

4.

Will this increase coupling?

5.

Can another provider replace Deepgram without changing this code?

If the answer to the last question is "no", reconsider the design.

---

# 32. Before Finishing a Task

Verify:

- project builds
- formatting is consistent
- responsibilities remain separated
- no secrets introduced
- interfaces preserved
- documentation updated if needed

---

# 33. Long-Term Vision

EchoLens is intended to evolve into a robust wearable assistive platform rather than a single-purpose transcription demo.

The architecture should support future capabilities—including offline transcription, multiple speech providers, live translation, AI-assisted interaction, and additional hardware—without requiring major rewrites of existing modules.

Every implementation decision should favor incremental evolution over short-term convenience.
