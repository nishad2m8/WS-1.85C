#include "Audio_ES8311.h"
#include "es8311.h"
#include "I2C_Driver.h"
#include <esp_log.h>

static const char* TAG = "Audio_ES8311";

Audio audio;
uint8_t Volume = 10;
bool audio_initialized = false;
static es8311_handle_t es8311_handle = NULL;

esp_err_t ES8311_Codec_Init() {
    ESP_LOGI(TAG, "Initializing ES8311...");

    es8311_handle = es8311_create(I2C_NUM_0, ES8311_I2C_ADDR);
    if (es8311_handle == NULL) {
        ESP_LOGE(TAG, "Failed to create ES8311 handle");
        return ESP_FAIL;
    }

    es8311_clock_config_t es_clk = {
        .mclk_inverted = false,
        .sclk_inverted = false,
        .mclk_from_mclk_pin = true,
        .mclk_frequency = ES8311_SAMPLE_RATE * ES8311_MCLK_MULTIPLE,
        .sample_frequency = ES8311_SAMPLE_RATE
    };

    esp_err_t ret = es8311_init(es8311_handle, &es_clk, ES8311_RESOLUTION_16, ES8311_RESOLUTION_16);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8311 init failed: %d", ret);
        return ret;
    }

    es8311_microphone_config(es8311_handle, false);
    es8311_voice_volume_set(es8311_handle, 50, NULL);

    ESP_LOGI(TAG, "ES8311 ready");
    return ESP_OK;
}

void Amp_Enable(bool enable) {
    pinMode(AMP_ENABLE_PIN, OUTPUT);
    digitalWrite(AMP_ENABLE_PIN, enable ? HIGH : LOW);
}

void ES8311_SetVolume(int volume) {
    if (es8311_handle != NULL) {
        es8311_voice_volume_set(es8311_handle, volume, NULL);
    }
}

esp_err_t Audio_Init() {
    ESP_LOGI(TAG, "Audio init...");

    Amp_Enable(true);
    delay(50);

    esp_err_t ret = ES8311_Codec_Init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8311 codec init failed");
        return ret;
    }

    audio.setBufsize(32000, 200000);

    if (!audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_MCLK)) {
        ESP_LOGE(TAG, "I2S pinout failed");
        return ESP_FAIL;
    }

    audio.setConnectionTimeout(500, 5000);
    audio.setVolume(Volume);

    int codec_volume = (Volume * 100) / Volume_MAX;
    ES8311_SetVolume(codec_volume);

    audio_initialized = true;
    ESP_LOGI(TAG, "Audio ready");

    return ESP_OK;
}

bool Is_Audio_Initialized() {
    return audio_initialized;
}

void Play_Music_test() {
    if (!audio_initialized) return;
    audio.connecttoFS(SD_MMC, "/test.mp3");
}

void Audio_Loop() {
    if (audio_initialized) {
        audio.loop();
    }
}

void Volume_adjustment(uint8_t vol) {
    if (vol > Volume_MAX) vol = Volume_MAX;
    Volume = vol;
    audio.setVolume(Volume);

    int codec_volume = (Volume * 100) / Volume_MAX;
    ES8311_SetVolume(codec_volume);
}

void Play_Music(const char* directory, const char* fileName) {
    if (!audio_initialized) return;

    char filePath[128];
    snprintf(filePath, sizeof(filePath), "%s/%s", directory, fileName);
    audio.connecttoFS(SD_MMC, filePath);
}

void Music_pause() {
    if (audio_initialized) {
        audio.pauseResume();
    }
}

void Music_resume() {
    if (audio_initialized) {
        audio.pauseResume();
    }
}

uint32_t Music_Duration() {
    if (!audio_initialized) return 0;
    return audio.getAudioFileDuration();
}

uint32_t Music_Elapsed() {
    if (!audio_initialized) return 0;
    return audio.getAudioCurrentTime();
}

uint16_t Music_Energy() {
    if (!audio_initialized) return 0;
    return audio.getVUlevel();
}

bool Play_Radio(const char* url) {
    if (!audio_initialized) return false;
    return audio.connecttohost(url);
}

void Stop_Radio() {
    if (audio_initialized) {
        audio.stopSong();
    }
}

bool Is_Radio_Playing() {
    if (!audio_initialized) return false;
    return audio.isRunning();
}
