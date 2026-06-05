#pragma once
#include "Arduino.h"
#include "Audio.h"
#include "SD_Card.h"
#include <esp_err.h>

// V2 Hardware - ES8311 Audio Codec
// I2S Pins
#define I2S_DOUT      47
#define I2S_DIN       39
#define I2S_BCLK      48
#define I2S_LRC       38
#define I2S_MCLK      2

// Amp control
#define AMP_ENABLE_PIN  15

// ES8311 config
#define ES8311_I2C_ADDR       0x18
#define ES8311_SAMPLE_RATE    44100
#define ES8311_MCLK_MULTIPLE  256

#define EXAMPLE_Audio_TICK_PERIOD_MS  20
#define Volume_MAX  21

extern Audio audio;
extern uint8_t Volume;
extern bool audio_initialized;

esp_err_t ES8311_Codec_Init();
esp_err_t Audio_Init();
bool Is_Audio_Initialized();
void Play_Music_test();
void Audio_Loop();
void Volume_adjustment(uint8_t Volume);
void Play_Music(const char* directory, const char* fileName);
void Music_pause();
void Music_resume();
uint32_t Music_Duration();
uint32_t Music_Elapsed();
uint16_t Music_Energy();
bool Play_Radio(const char* url);
void Stop_Radio();
bool Is_Radio_Playing();
void Amp_Enable(bool enable);
void ES8311_SetVolume(int volume);
