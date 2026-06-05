#include "Audio_PCM5101.h"
#include <WiFi.h>

Audio audio;
uint8_t Volume = Volume_MAX;
bool audio_initialized = false;

void IRAM_ATTR example_increase_audio_tick(void *arg)
{
  if (audio_initialized) {
    audio.loop();
  }
}

esp_err_t Audio_Init() {
  audio_initialized = false;

  // Set buffers for streaming (RAM: 32KB, PSRAM: 200KB)
  audio.setBufsize(32000, 200000);

  // Audio pins configuration
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  // Set connection timeout: 500ms for HTTP, 5000ms for HTTPS
  audio.setConnectionTimeout(500, 5000);

  // Set balanced tone and stereo
  audio.setTone(0, 0, 0);
  audio.setBalance(0);
  audio.setVolume(21);

  // Create audio tick timer
  esp_timer_handle_t audio_tick_timer = NULL;
  const esp_timer_create_args_t audio_tick_timer_args = {
    .callback = &example_increase_audio_tick,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "audio_tick",
    .skip_unhandled_events = true
  };

  if (esp_timer_create(&audio_tick_timer_args, &audio_tick_timer) != ESP_OK) {
    return ESP_FAIL;
  }

  if (esp_timer_start_periodic(audio_tick_timer, EXAMPLE_Audio_TICK_PERIOD_MS * 1000) != ESP_OK) {
    return ESP_FAIL;
  }

  audio_initialized = true;
  return ESP_OK;
}

void Volume_adjustment(uint8_t Volume) {
  if (!audio_initialized) return;
  if (Volume <= Volume_MAX) {
    audio.setVolume(Volume);
  }
}

void Play_Music_test() {
  if (!audio_initialized || !SD_IsAvailable()) return;

  if (SD_MMC.exists("/A.mp3")) {
    audio.connecttoFS(SD_MMC, "/A.mp3");
  }
}

void Play_Music(const char* directory, const char* fileName) {
  // Safety checks
  if (!audio_initialized) {
    Serial.println("Audio not initialized");
    return;
  }

  if (!SD_IsAvailable()) {
    Serial.println("SD card not available");
    return;
  }

  // Check file exists
  if (!File_Search(directory, fileName)) {
    return;
  }

  const int maxPathLength = 100;
  char filePath[maxPathLength];
  if (strcmp(directory, "/") == 0) {
    snprintf(filePath, maxPathLength, "%s%s", directory, fileName);
  } else {
    snprintf(filePath, maxPathLength, "%s/%s", directory, fileName);
  }

  // Stop current playback and connect to new file
  audio.stopSong();
  audio.connecttoFS(SD_MMC, (char*)filePath);
}

void Music_pause() {
  if (!audio_initialized) return;
  if (audio.isRunning()) {
    audio.pauseResume();
  }
}

void Music_resume() {
  if (!audio_initialized) return;
  if (!audio.isRunning()) {
    audio.pauseResume();
  }
}

uint32_t Music_Duration() {
  if (!audio_initialized) return 0;
  return audio.getAudioFileDuration();
}

uint32_t Music_Elapsed() {
  if (!audio_initialized) return 0;
  
  uint32_t Audio_elapsed = audio.getAudioCurrentTime(); 
  return Audio_elapsed;
}

uint16_t Music_Energy() {
  if (!audio_initialized) return 0;
  
  uint16_t Audio_Energy = audio.getVUlevel(); 
  return Audio_Energy;
}

void Audio_Loop() {
  if (!audio_initialized) return;
  
  if (!audio.isRunning())
    Play_Music_test();
}

bool Is_Audio_Initialized() {
  return audio_initialized;
}

// Radio streaming implementation
static bool radio_playing = false;

bool Play_Radio(const char* url) {
  if (!audio_initialized || !WiFi.isConnected()) {
    return false;
  }

  bool ret = audio.connecttohost(url);
  radio_playing = ret;
  return ret;
}

void Stop_Radio() {
  if (!audio_initialized) return;
  audio.stopSong();
  radio_playing = false;
}

bool Is_Radio_Playing() {
  return radio_playing && audio.isRunning();
}