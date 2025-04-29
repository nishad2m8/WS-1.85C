#include "SD_Card.h"

bool SDCard_Flag = false;
bool SDCard_Finish = false;

uint16_t SDCard_Size = 0;
uint16_t Flash_Size = 0;

void SD_D3_Dis(){
  Set_EXIO(EXIO_PIN4, Low);
  vTaskDelay(pdMS_TO_TICKS(10));
}

void SD_D3_EN(){
  Set_EXIO(EXIO_PIN4, High);
  vTaskDelay(pdMS_TO_TICKS(10));
}

esp_err_t SD_Init() {
  // Initialize flags to a safe state
  SDCard_Flag = false;
  SDCard_Finish = false;
  
  // SD MMC
  if(!SD_MMC.setPins(SD_CLK_PIN, SD_CMD_PIN, SD_D0_PIN, -1, -1, -1)){
    printf("SD MMC: Pin change failed!\r\n");
    return ESP_FAIL;
  }
  
  // Enable SD card D3 line
  SD_D3_EN();
  
  // Try to mount the SD card with a reasonable timeout
  int retry_count = 0;
  bool sd_initialized = false;
  
  while (retry_count < 3 && !sd_initialized) {
    sd_initialized = SD_MMC.begin("/sdcard", true, true);
    if (!sd_initialized) {
      printf("SD init attempt %d failed, retrying...\r\n", retry_count + 1);
      vTaskDelay(pdMS_TO_TICKS(100)); // Wait before retry
      retry_count++;
    }
  }
  
  if (sd_initialized) {
    printf("SD card initialization successful!\r\n");
  } else {
    printf("SD card initialization failed after %d attempts!\r\n", retry_count);
    return ESP_FAIL;
  }
  
  // Check card type
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    printf("No SD card attached\r\n");
    SD_MMC.end(); // Clean up if no card
    return ESP_FAIL;
  }
  
  // Card is present and initialized
  printf("SD Card Type: ");
  if(cardType == CARD_MMC){
    printf("MMC\r\n");
  } else if(cardType == CARD_SD){
    printf("SDSC\r\n");
  } else if(cardType == CARD_SDHC){
    printf("SDHC\r\n");
  } else {
    printf("UNKNOWN\r\n");
  }
  
  // Get and display card information
  uint64_t totalBytes = SD_MMC.totalBytes();
  uint64_t usedBytes = SD_MMC.usedBytes();
  SDCard_Size = totalBytes/(1024*1024);
  printf("Total space: %llu MB\r\n", totalBytes/(1024*1024));
  printf("Used space: %llu MB\r\n", usedBytes/(1024*1024));
  printf("Free space: %llu MB\r\n", (totalBytes - usedBytes)/(1024*1024));
  
  // Set flags indicating successful initialization
  SDCard_Flag = true;
  SDCard_Finish = true;
  
  return ESP_OK;
}

bool SD_IsAvailable() {
  return SDCard_Flag && SD_MMC.cardType() != CARD_NONE;
}

void SD_End() {
  if (SDCard_Flag) {
    SD_MMC.end();
    SDCard_Flag = false;
  }
}

bool File_Search(const char* directory, const char* fileName) {
  // Check if SD is available before proceeding
  if (!SD_IsAvailable()) {
    printf("SD card not available for file search\r\n");
    return false;
  }
  
  File Path = SD_MMC.open(directory);
  if (!Path) {
    printf("Path: <%s> does not exist\r\n", directory);
    return false;
  }
  
  File file = Path.openNextFile();
  while (file) {
    if (strcmp(file.name(), fileName) == 0) {
      if (strcmp(directory, "/") == 0)
        printf("File '%s%s' found in root directory.\r\n", directory, fileName);
      else
        printf("File '%s/%s' found in root directory.\r\n", directory, fileName);
      Path.close();
      return true;
    }
    file = Path.openNextFile();
  }
  
  if (strcmp(directory, "/") == 0)
    printf("File '%s%s' not found in root directory.\r\n", directory, fileName);
  else
    printf("File '%s/%s' not found in root directory.\r\n", directory, fileName);
  
  Path.close();
  return false;
}

uint16_t Folder_retrieval(const char* directory, const char* fileExtension, char File_Name[][100], uint16_t maxFiles) {
  // Check if SD is available before proceeding
  if (!SD_IsAvailable()) {
    printf("SD card not available for folder retrieval\r\n");
    return 0;
  }
  
  File Path = SD_MMC.open(directory);
  if (!Path) {
    printf("Path: <%s> does not exist\r\n", directory);
    return 0;
  }
  
  uint16_t fileCount = 0;
  char filePath[100];
  File file = Path.openNextFile();
  
  while (file && fileCount < maxFiles) {
    if (!file.isDirectory() && strstr(file.name(), fileExtension)) {
      strncpy(File_Name[fileCount], file.name(), 99);
      File_Name[fileCount][99] = '\0'; // Ensure null termination
      
      if (strcmp(directory, "/") == 0) {
        snprintf(filePath, 100, "%s%s", directory, file.name());
      } else {
        snprintf(filePath, 100, "%s/%s", directory, file.name());
      }
      
      printf("File found: %s\r\n", filePath);
      fileCount++;
    }
    file = Path.openNextFile();
  }
  
  Path.close();
  
  if (fileCount > 0) {
    printf("Retrieved %d %s files\r\n", fileCount, fileExtension);
    return fileCount;
  } else {
    printf("No files with extension '%s' found in directory: %s\r\n", fileExtension, directory);
    return 0;
  }
}

void Flash_test() {
  printf("/********** RAM Test**********/\r\n");
  // Get Flash size
  uint32_t flashSize = ESP.getFlashChipSize();
  Flash_Size = flashSize/1024/1024;
  printf("Flash size: %d MB \r\n", Flash_Size);
  printf("/******* RAM Test Over********/\r\n\r\n");
}