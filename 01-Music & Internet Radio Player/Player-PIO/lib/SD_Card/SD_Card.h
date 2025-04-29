#pragma once
#include "Arduino.h"
#include <cstring>
#include "FS.h"
#include "SD_MMC.h"
#include <esp_err.h>

#include "TCA9554PWR.h"

#define SD_CLK_PIN      14
#define SD_CMD_PIN      17 
#define SD_D0_PIN       16 

extern bool SDCard_Flag;
extern bool SDCard_Finish;
extern uint16_t SDCard_Size;
extern uint16_t Flash_Size;

// Initialize SD card - returns ESP_OK on success, ESP_FAIL on failure
esp_err_t SD_Init();

// Check if SD card is available and initialized
bool SD_IsAvailable();

// End SD card operations and release resources
void SD_End();

// Flash memory test function
void Flash_test();

// Search for a specific file in a directory
bool File_Search(const char* directory, const char* fileName);

// Retrieve files with specific extension from a directory
uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][100], uint16_t maxFiles);