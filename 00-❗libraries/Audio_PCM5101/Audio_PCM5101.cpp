#include "Audio_PCM5101.h"

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
  // Set initial state
  audio_initialized = false;

  try {
    // Audio pins configuration
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(21); // 0...21    

    esp_timer_handle_t audio_tick_timer = NULL;
    const esp_timer_create_args_t audio_tick_timer_args = {
      .callback = &example_increase_audio_tick,
      .dispatch_method = ESP_TIMER_TASK,  
      .name = "audio_tick",
      .skip_unhandled_events = true       
    };
    
    esp_err_t timer_result = esp_timer_create(&audio_tick_timer_args, &audio_tick_timer);
    if (timer_result != ESP_OK) {
      Serial.printf("Failed to create audio timer: %d\n", timer_result);
      return ESP_FAIL;
    }
    
    timer_result = esp_timer_start_periodic(audio_tick_timer, EXAMPLE_Audio_TICK_PERIOD_MS * 1000);
    if (timer_result != ESP_OK) {
      Serial.printf("Failed to start audio timer: %d\n", timer_result);
      return ESP_FAIL;
    }
    
    // Mark as successfully initialized
    audio_initialized = true;
    Serial.println("Audio initialized successfully");
    return ESP_OK;
  } 
  catch (...) {
    Serial.println("Error during audio initialization");
    return ESP_FAIL;
  }
}

void Volume_adjustment(uint8_t Volume) {
  if (!audio_initialized) {
    Serial.println("Audio not initialized, cannot adjust volume");
    return;
  }

  if(Volume > Volume_MAX )
    printf("Audio : The volume value is incorrect. Please enter 0 to 21\r\n");
  else
    audio.setVolume(Volume); // 0...21    
}

void Play_Music_test() {
  // Check if audio and SD card are available
  if (!audio_initialized) {
    Serial.println("Audio not initialized, cannot play music");
    return;
  }
  
  if (!SD_IsAvailable()) {
    Serial.println("SD card not available, cannot play music");
    return;
  }
  
  // SD Card
  if (SD_MMC.exists("/A.mp3")) {
    printf("File 'A.mp3' found in root directory.\r\n");
  } else {
    printf("File 'A.mp3' not found in root directory.\r\n");
    return;
  }
  
  bool ret = audio.connecttoFS(SD_MMC,"/A.mp3");
  if(ret) 
    printf("Music Read OK\r\n");
  else
    printf("Music Read Failed\r\n");
}

void Play_Music(const char* directory, const char* fileName) {
  // Safety checks
  if (!audio_initialized) {
    Serial.println("Audio not initialized, cannot play music");
    return;
  }
  
  if (!SD_IsAvailable()) {
    Serial.println("SD card not available, cannot play music");
    return;
  }
  
  // SD Card
  if (!File_Search(directory, fileName)) {
    printf("%s file not found.\r\n", fileName);
    return;
  }
  
  const int maxPathLength = 100; 
  char filePath[maxPathLength];
  if (strcmp(directory, "/") == 0) {                                               
    snprintf(filePath, maxPathLength, "%s%s", directory, fileName);   
  } else {                                                            
    snprintf(filePath, maxPathLength, "%s/%s", directory, fileName);
  }
  
  // Safely play the file
  audio.pauseResume();     
  bool ret = audio.connecttoFS(SD_MMC, (char*)filePath);
  if(ret) 
    printf("Music Read OK\r\n");
  else
    printf("Music Read Failed\r\n");
  
  Music_pause();           
  Music_resume();               
  Music_pause();     
  vTaskDelay(pdMS_TO_TICKS(100));    
}

void Music_pause() {
  if (!audio_initialized) return;
  
  if (audio.isRunning()) {            
    audio.pauseResume();             
    printf("The music pause\r\n");
  }
}

void Music_resume() {
  if (!audio_initialized) return;
  
  if (!audio.isRunning()) {           
    audio.pauseResume();             
    printf("The music begins\r\n");
  } 
}

uint32_t Music_Duration() {
  if (!audio_initialized) return 0;
  
  uint32_t Audio_duration = audio.getAudioFileDuration(); 
  // Audio_duration = 360;
  if(Audio_duration > 60)
    printf("Audio duration is %d minutes and %d seconds\r\n", Audio_duration/60, Audio_duration%60);
  else{
    if(Audio_duration != 0)
      printf("Audio duration is %d seconds\r\n", Audio_duration);
    else
      printf("Fail : Failed to obtain the audio duration.\r\n");
  }
  vTaskDelay(pdMS_TO_TICKS(10));
  return Audio_duration;
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