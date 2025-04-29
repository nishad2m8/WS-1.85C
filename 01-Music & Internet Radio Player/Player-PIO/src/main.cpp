#include <Arduino.h>
#include "Display_ST77916.h"
#include "Audio_PCM5101.h"
#include "LVGL_Driver.h"
#include "BAT_Driver.h"
#include "SD_Card.h"
#include "WiFiManager.h"

#include "ui.h"  // LVGL UI initialization
#include "UIController.h"
#include "AudioPlayer.h"

// Flag to track SD card status
bool sd_card_available = false;

// Task handles
TaskHandle_t ui_task_handle = NULL;
TaskHandle_t driver_task_handle = NULL;

// Add watchdog timer functions
#include "esp_task_wdt.h"

void Driver_Loop(void *parameter)
{
    // Register this task with the watchdog - newer API
    esp_task_wdt_add(NULL);
    
    // Main driver loop
    while (1)
    {
        // Reset the watchdog timer
        esp_task_wdt_reset();
        
        // Regular system tasks - use correct function name
        BAT_Get_Volts(); // Keep original function name if this is correct
        
        // Periodically check WiFi connection
        static uint32_t lastWifiCheck = 0;
        if (millis() - lastWifiCheck > 60000) { // Check every minute (reduced frequency)
            if (!WiFi_IsConnected()) {
                WiFi_Reconnect();
            }
            lastWifiCheck = millis();
        }
        
        // More time between iterations to save CPU
        vTaskDelay(pdMS_TO_TICKS(250)); // Increased from 200ms
    }
}

void UI_Task(void *parameter)
{
    // Register this task with the watchdog - newer API
    esp_task_wdt_add(NULL);
    
    while (1)
    {
        // Reset the watchdog timer
        esp_task_wdt_reset();
        
        // Update the LVGL UI - use try/catch to prevent crashes
        try {
            Lvgl_Loop();
            
            // Process UI controller events
            UIController_Loop();
        } catch (...) {
            Serial.println("Error in UI Loop - caught exception");
        }
        
        // Keep fixed delay to prevent watchdog timeouts
        vTaskDelay(pdMS_TO_TICKS(10)); // Increased from 5ms
    }
}

void Driver_Init()
{
    BAT_Init();
    I2C_Init();
    TCA9554PWR_Init(0x00);
    Backlight_Init();
}

void memory_info() {
    // Print current memory stats
    Serial.printf("Heap info - Total: %d, Free: %d, Min Free: %d\n", 
                 ESP.getHeapSize(), ESP.getFreeHeap(), ESP.getMinFreeHeap());
                 
    #ifdef CONFIG_SPIRAM_SUPPORT
    // Only print PSRAM info if enabled
    if (esp_spiram_is_initialized()) {
        Serial.printf("PSRAM info - Total: %d, Free: %d, Min Free: %d\n", 
                     ESP.getPsramSize(), ESP.getFreePsram(), ESP.getMinFreePsram());
    } else {
        Serial.println("PSRAM is enabled in menuconfig but not initialized!");
    }
    #endif
}

// Memory-optimized setup function
void setup()
{
    // Initialize serial communication first for debugging
    Serial.begin(115200);
    delay(100);
    
    Serial.println("Starting audio player setup with PSRAM support...");
    
    // Configure watchdog timer with updated API
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 10000,            // 10 seconds timeout
        .idle_core_mask = (1 << 0),     // Bitmask of cores that can trigger idle timeout
        .trigger_panic = true           // Trigger panic on timeout
    };
    esp_task_wdt_init(&wdt_config);
    
    // Print initial memory info
    memory_info();
    
    // Initialize basic drivers
    Driver_Init();
    
    #ifdef CONFIG_SPIRAM_SUPPORT
    if (esp_spiram_is_initialized()) {
        Serial.println("PSRAM is available for audio buffers");
    } else {
        Serial.println("Warning: PSRAM not initialized despite being enabled");
    }
    #endif
    
    // Pre-initialize audio with proper configuration
    if (audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
        Serial.println("Audio hardware initialized successfully");
    } else {
        Serial.println("Audio hardware initialization failed");
    }
    
    // Initialize the LCD and backlight with reduced power
    LCD_Init();
    Set_Backlight(40);
    
    // Initialize LVGL with memory optimizations
    Lvgl_Init();

    // Initialize UI components
    ui_init();

    // Initialize SD card with proper error handling
    Serial.println("Initializing SD card...");
    esp_err_t sd_result = SD_Init();
    if (sd_result == ESP_OK) {
        Serial.println("SD card initialized successfully");
        sd_card_available = true;
        
        // Minimal SD card check to save memory
        File root = SD_MMC.open("/");
        if (root && root.isDirectory()) {
            File file = root.openNextFile();
            if (file) {
                Serial.printf("Found file: %s\n", file.name());
                file.close();
            } else {
                Serial.println("No files found in root directory");
            }
            root.close();
        }
    } else {
        Serial.println("SD card initialization failed, continuing without SD card");
        sd_card_available = false;
    }
    
    // Memory checkpoint before WiFi
    memory_info();
    
    // Initialize WiFi connection - do this after SD card to improve memory
    Serial.println("Initializing WiFi...");
    WiFi_Init();
    
    // Memory checkpoint after WiFi
    memory_info();
    
    // Create UI task on core 0 with increased stack size
    // The stack overflow was in UITask, so we're increasing its size
    xTaskCreatePinnedToCore(
        UI_Task,
        "UITask",
        8192,       // Increased from 4096 to 8192 to prevent stack overflow
        NULL,
        2,          // Priority 2
        &ui_task_handle,
        0           // Core 0 for UI
    );
    
    // Driver task on core 0 with minimal stack
    xTaskCreatePinnedToCore(
        Driver_Loop,
        "DriverTask",
        2048,       // Same size as before
        NULL,
        1,          // Priority 1 (lower than UI task)
        &driver_task_handle,
        0           // Core 0
    );
    
    // Wait before initializing the audio subsystem
    vTaskDelay(pdMS_TO_TICKS(200));
    
    // Memory checkpoint before UI controller
    memory_info();
    
    // Initialize the UI controller
    UIController_Init();
    
    // Final memory check
    memory_info();
    
    Serial.println("Setup complete");
}

void loop()
{
    // The main loop should be empty since we're using FreeRTOS tasks
    // Just delay to keep the loop task from using CPU
    vTaskDelay(pdMS_TO_TICKS(1000));
}