# EchoLens architecture notes

## Module boundaries

- `audio/` never includes anything from `network/`, `display/`, or
  `transcription/`. It produces `int16_t` PCM samples and nothing else.
- `network/` provides generic WiFi + secure WebSocket primitives with no
  Deepgram-specific framing.
- `transcription/` owns the `TranscriptionProvider` interface. Only this
  layer knows about Deepgram's message schema. It receives a provider-neutral
  `system::WebSocketTransport` interface rather than depending on `network/`.
- `display/` is split into `OledDisplay` (hardware primitives) and
  `TranscriptRenderer` (word wrap / scrolling), so a future e-ink panel or
  larger OLED only requires a new `OledDisplay`-compatible class.

## Data flow

```
INMP441 --I2S--> Microphone --samples--> AudioRingBuffer
                                              |
                                     NetworkTask reads chunks
                                              |
                         WebSocketTransport -> DeepgramProvider.sendAudio()
                                              |
                                  Deepgram Streaming API (wss)
                                              |
                                   JSON transcript response
                                              |
                                   parser::ParseDeepgramResponse
                                              |
                                 TranscriptQueue (FreeRTOS queue)
                                              |
                                DisplayTask -> TranscriptRenderer -> OLED
```

## Why a ring buffer instead of a raw queue of samples

Audio capture and network transmission run at different, independently
jittery rates (I2S DMA cadence vs. WiFi/TLS scheduling). A ring buffer
decouples producer and consumer cleanly, with bounded memory and explicit
overrun handling (oldest samples are dropped, never silently corrupted).

## Known follow-ups

- Persist Deepgram session state across reconnects (currently a full
  WebSocket re-handshake).
- Add exponential backoff to `WifiManager::poll()` (currently fixed
  interval).
- Profile the fixed `StaticJsonDocument` capacity in `parser.cpp` against real
  Deepgram payloads once smart-format / diarization features are enabled.
