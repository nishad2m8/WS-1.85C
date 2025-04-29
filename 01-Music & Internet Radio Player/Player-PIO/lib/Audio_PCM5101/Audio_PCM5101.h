#pragma once
#include "Arduino.h"
#include "AudioConfig.h"
#include "Audio.h"
#include "SD_Card.h"
#include "MIC_MSM.h"
#include <esp_err.h>

// Digital I/O used
#define I2S_DOUT      47
#define I2S_BCLK      48  
#define I2S_LRC       38      // I2S_WS

#define EXAMPLE_Audio_TICK_PERIOD_MS  20
#define Volume_MAX  21

extern Audio audio;
extern uint8_t Volume;
extern bool audio_initialized;

// Initialize audio system
esp_err_t Audio_Init();

// Check if audio is initialized
bool Is_Audio_Initialized();

// Play test music file
void Play_Music_test();

// Audio continuous operation
void Audio_Loop();

// Adjust volume (0-21)
void Volume_adjustment(uint8_t Volume);

// Play specific music file
void Play_Music(const char* directory, const char* fileName);

// Control functions
void Music_pause(); 
void Music_resume();    

// Status functions
uint32_t Music_Duration();  
uint32_t Music_Elapsed();   
uint16_t Music_Energy();