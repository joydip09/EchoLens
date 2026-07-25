#include "microphone.h"

#include <driver/i2s.h>

#include "../system/logger.h"
#include "../../include/constants.h"

namespace echolens::audio {

namespace {
constexpr const char* kTag = "Microphone";
constexpr i2s_port_t kI2sPort = I2S_NUM_0;
}  // namespace

Microphone::Microphone(const MicrophoneConfig& config) : config_(config) {}

bool Microphone::begin() {
    const i2s_config_t i2sConfig = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = config_.sampleRateHz,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // INMP441 outputs 24-bit in a 32-bit frame
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = constants::kI2sDmaBufCount,
        .dma_buf_len = constants::kI2sDmaBufLen,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    const i2s_pin_config_t pinConfig = {
        .bck_io_num = config_.bclkPin,
        .ws_io_num = config_.wsPin,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = config_.dataInPin,
    };

    if (i2s_driver_install(kI2sPort, &i2sConfig, 0, nullptr) != ESP_OK) {
        system::Logger::error(kTag, "i2s_driver_install failed");
        return false;
    }
    if (i2s_set_pin(kI2sPort, &pinConfig) != ESP_OK) {
        system::Logger::error(kTag, "i2s_set_pin failed");
        return false;
    }

    i2s_zero_dma_buffer(kI2sPort);
    running_ = true;
    system::Logger::info(kTag, "I2S microphone initialized at %u Hz", config_.sampleRateHz);
    return true;
}

size_t Microphone::readSamples(int16_t* out, size_t maxSamples) {
    if (!running_ || out == nullptr || maxSamples == 0) {
        return 0;
    }

    // The INMP441 delivers 24-bit samples left-justified in a 32-bit slot.
    // Read into a stack scratch buffer sized to a bounded chunk, then
    // downshift to 16-bit signed samples the rest of the firmware expects.
    static constexpr size_t kScratchFrames = 256;
    int32_t scratch[kScratchFrames];

    size_t samplesRemaining = maxSamples;
    size_t totalRead = 0;

    while (samplesRemaining > 0 && totalRead < maxSamples) {
        size_t framesToRead = samplesRemaining < kScratchFrames ? samplesRemaining : kScratchFrames;
        size_t bytesToRead = framesToRead * sizeof(int32_t);
        size_t bytesRead = 0;

        esp_err_t err = i2s_read(kI2sPort, scratch, bytesToRead, &bytesRead, pdMS_TO_TICKS(50));
        if (err != ESP_OK) {
            system::Logger::warning(kTag, "i2s_read error: %d", static_cast<int>(err));
            break;
        }
        if (bytesRead == 0) {
            break;  // no data ready within timeout
        }

        size_t framesRead = bytesRead / sizeof(int32_t);
        for (size_t i = 0; i < framesRead; ++i) {
            // Shift the 24-bit sample (top-justified) down to 16-bit range.
            out[totalRead + i] = static_cast<int16_t>(scratch[i] >> 16);
        }

        totalRead += framesRead;
        samplesRemaining -= framesRead;
    }

    return totalRead;
}

void Microphone::stop() {
    if (running_) {
        i2s_driver_uninstall(kI2sPort);
        running_ = false;
    }
}

}  // namespace echolens::audio
