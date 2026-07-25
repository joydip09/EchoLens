#include <Arduino.h>
#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "../include/config.h"
#include "../include/constants.h"
#include "../include/pins.h"
#include "../include/version.h"

#include "audio/microphone.h"
#include "display/oled.h"
#include "display/boot_animation.h"
#include "display/renderer.h"
#include "network/wifi.h"
#include "network/websocket.h"
#include "system/logger.h"
#include "system/ringbuffer.h"
#include "transcription/deepgram.h"
#include "transcription/parser.h"

// -----------------------------------------------------------------------
// Application composition root. Wires the independent modules together
// and owns the FreeRTOS tasks + inter-task communication primitives.
// Individual modules never reach into each other directly - everything
// flows through the ring buffer, the transcript queue, or callbacks
// configured here.
// -----------------------------------------------------------------------

using namespace echolens;

namespace {

constexpr const char* kTag = "Main";

AppConfig g_config;

audio::Microphone g_microphone(audio::MicrophoneConfig{
    .bclkPin = pins::kI2sBclk,
    .wsPin = pins::kI2sWs,
    .dataInPin = pins::kI2sDin,
    .sampleRateHz = constants::kSampleRateHz,
});

system::AudioRingBuffer g_audioRing(constants::kRingBufferSamples);

network::WifiManager g_wifi(g_config.wifi.ssid, g_config.wifi.password,
                             g_config.wifi.connectTimeoutMs);

network::SecureWebSocketClient g_webSocket;
transcription::DeepgramProvider g_transcriptionProvider(g_config.deepgram, g_webSocket);

display::OledDisplay g_oled(display::OledConfig{
    .width = constants::kOledWidth,
    .height = constants::kOledHeight,
    .i2cAddress = constants::kOledAddress,
    .sdaPin = pins::kI2cSda,
    .sclPin = pins::kI2cScl,
});

display::TranscriptRenderer g_renderer(g_oled);

// Transcript results flow from the network task to the display task
// through this queue - the display task never touches the network stack.
QueueHandle_t g_transcriptQueue = nullptr;

struct TranscriptMessage {
    char text[constants::kMaxTranscriptChars];
    bool isFinal;
    bool speechFinal;
};

// -----------------------------------------------------------------------
// Task bodies
// -----------------------------------------------------------------------

void AudioTask(void* /*param*/) {
    system::Logger::info(kTag, "AudioTask started");

    static constexpr size_t kReadChunk = constants::kI2sDmaBufLen;
    int16_t scratch[kReadChunk];

    for (;;) {
        size_t samplesRead = g_microphone.readSamples(scratch, kReadChunk);
        if (samplesRead > 0) {
            g_audioRing.write(scratch, samplesRead);
        } else {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void NetworkTask(void* /*param*/) {
    system::Logger::info(kTag, "NetworkTask started");

    g_transcriptionProvider.onTranscript([](const transcription::TranscriptResult& result) {
        TranscriptMessage msg{};
        strncpy(msg.text, result.text.c_str(), sizeof(msg.text) - 1);
        msg.isFinal = result.isFinal;
        msg.speechFinal = result.speechFinal;
        if (g_transcriptQueue != nullptr &&
            xQueueSend(g_transcriptQueue, &msg, 0) != pdTRUE) {
            system::Logger::warning(kTag, "Transcript queue full; dropping update");
        }
    });

    g_transcriptionProvider.begin();

    int16_t audioChunk[constants::kAudioChunkSamples];

    for (;;) {
        g_wifi.poll();
        g_transcriptionProvider.poll();

        if (g_wifi.isConnected() && g_transcriptionProvider.isConnected()) {
            // Drain every complete PCM frame already queued. Sending only one
            // frame per scheduler pass made the producer outrun this task.
            while (g_audioRing.availableToRead() >= constants::kAudioChunkSamples) {
                size_t read = g_audioRing.read(audioChunk, constants::kAudioChunkSamples);
                if (read != constants::kAudioChunkSamples ||
                    !g_transcriptionProvider.sendAudio(audioChunk, read)) {
                    break;
                }
                // Service incoming frames between outgoing packets so a burst
                // of captured audio cannot starve WebSocket receive handling.
                g_transcriptionProvider.poll();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void DisplayTask(void* /*param*/) {
    system::Logger::info(kTag, "DisplayTask started");

    TranscriptMessage msg;
    for (;;) {
        if (xQueueReceive(g_transcriptQueue, &msg, pdMS_TO_TICKS(200)) == pdTRUE) {
            if (msg.isFinal) {
                g_renderer.commitFinalText(msg.text, msg.speechFinal);
            } else {
                g_renderer.setInterimText(msg.text);
            }
        }
        g_renderer.render();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void SystemTask(void* /*param*/) {
    system::Logger::info(kTag, "SystemTask started");

    for (;;) {
        const size_t droppedSamples = g_audioRing.takeDroppedSamples();
        if (droppedSamples > 0) {
            system::Logger::warning(kTag, "Audio overrun: dropped %u samples",
                                    static_cast<unsigned>(droppedSamples));
        }
        system::Logger::debug(kTag, "Heap free: %u bytes, min free: %u bytes",
                               static_cast<unsigned>(ESP.getFreeHeap()),
                               static_cast<unsigned>(ESP.getMinFreeHeap()));
        // TODO: future watchdog / health-check hooks live here.
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

bool StartTask(TaskFunction_t task, const char* name, uint32_t stackWords,
               UBaseType_t priority, BaseType_t core) {
    if (xTaskCreatePinnedToCore(task, name, stackWords, nullptr, priority, nullptr, core) != pdPASS) {
        system::Logger::error(kTag, "Failed to start %s", name);
        return false;
    }
    return true;
}

}  // namespace

void setup() {
    system::Logger::begin(115200);
    system::Logger::info(kTag, "EchoLens v%s booting", version::kString);

    const bool displayReady = g_oled.begin();
    if (!displayReady) {
        system::Logger::error(kTag, "OLED init failed; continuing without display");
    } else {
        display::BootAnimation bootAnimation(g_oled);
        bootAnimation.play();
        g_renderer.setInterimText("EchoLens ready. Listening...");
        g_renderer.render();
    }

    const bool microphoneReady = g_microphone.begin();
    if (!microphoneReady) {
        system::Logger::error(kTag, "Microphone init failed; audio pipeline disabled");
    }

    if (displayReady) {
        g_transcriptQueue = xQueueCreate(constants::kTranscriptQueueLength, sizeof(TranscriptMessage));
    }
    if (displayReady && g_transcriptQueue == nullptr) {
        system::Logger::error(kTag, "Failed to create transcript queue");
    }

    if (microphoneReady) {
        StartTask(AudioTask, "AudioTask", constants::kAudioTaskStackWords,
                  constants::kAudioTaskPriority, 1);
    }
    StartTask(NetworkTask, "NetworkTask", constants::kNetworkTaskStackWords,
              constants::kNetworkTaskPriority, 0);
    if (g_transcriptQueue != nullptr) {
        StartTask(DisplayTask, "DisplayTask", constants::kDisplayTaskStackWords,
                  constants::kDisplayTaskPriority, 0);
    }
    StartTask(SystemTask, "SystemTask", constants::kSystemTaskStackWords,
              constants::kSystemTaskPriority, 0);

    system::Logger::info(kTag, "Task startup sequence complete");
}

void loop() {
    // Intentionally empty - all work happens in FreeRTOS tasks.
    vTaskDelay(pdMS_TO_TICKS(1000));
}
