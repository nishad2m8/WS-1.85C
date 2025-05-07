#pragma once
#include "ESP_I2S.h"
#include "ESP_SR.h"
#include "Display_ST77916.h"
#include "esp_task_wdt.h"

#define I2S_PIN_BCK   15
#define I2S_PIN_WS    2
#define I2S_PIN_DOUT  -1
#define I2S_PIN_DIN   39


// #define I2S_PIN_BCK   48
// #define I2S_PIN_WS    38
// #define I2S_PIN_DOUT  47
// #define I2S_PIN_DIN   39

void MIC_Init(void);