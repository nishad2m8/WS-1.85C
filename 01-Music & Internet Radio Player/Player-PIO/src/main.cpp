#include <Arduino.h>
#include "config.h"
#include "Display_ST77916.h"

#ifdef HARDWARE_V2
#include "Audio_ES8311.h"
#else
#include "Audio_PCM5101.h"
#endif

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

        // Regular system tasks
        BAT_Get_Volts();

        // Update WiFi status (non-blocking check)
        WiFi_Update();

        // Periodically trigger reconnection if needed
        static uint32_t lastWifiCheck = 0;
        if (millis() - lastWifiCheck > 60000) { // Check every minute
            if (!WiFi_IsConnected()) {
                WiFi_Reconnect();
            }
            lastWifiCheck = millis();
        }

        vTaskDelay(pdMS_TO_TICKS(250));
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

// Memory-optimized setup function with boot priority optimization
void setup()
{
    // Initialize serial communication first for debugging
    Serial.begin(115200);
    delay(50); // Reduced from 100ms

    Serial.println("Boot starting...");

    // Configure watchdog timer - increased timeout for boot phase
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 15000,            // 15 seconds for boot (increased safety margin)
        .idle_core_mask = (1 << 0),     // Bitmask of cores that can trigger idle timeout
        .trigger_panic = true           // Trigger panic on timeout
    };
    esp_task_wdt_init(&wdt_config);

    // PRIORITY 1: Get display working first (user feedback)
    Serial.println("Init drivers...");
    Driver_Init();
    esp_task_wdt_reset(); // Feed watchdog
    
    #ifdef CONFIG_SPIRAM_SUPPORT
    if (esp_spiram_is_initialized()) {
        Serial.println("PSRAM available");
    }
    #endif

    // PRIORITY 2: Display - user sees boot progress
    Serial.println("Init display...");
    LCD_Init();
    Set_Backlight(40);
    esp_task_wdt_reset(); // Feed watchdog

    // Initialize LVGL with memory optimizations
    Lvgl_Init();
    esp_task_wdt_reset(); // Feed watchdog

    // Initialize UI components - user sees something!
    ui_init();
    esp_task_wdt_reset(); // Feed watchdog
    Serial.println("Display ready");

    // PRIORITY 3: Audio hardware (fast init)
#ifdef HARDWARE_V2
    // V2: Initialize ES8311 codec with amplifier
    if (Audio_Init() == ESP_OK) {
        Serial.println("Audio ES8311 ready");
    } else {
        Serial.println("Audio ES8311 init failed");
    }
#else
    // V1: Simple I2S DAC setup
    if (audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
        Serial.println("Audio PCM5101 ready");
    }
#endif
    esp_task_wdt_reset(); // Feed watchdog

    // PRIORITY 4: SD card (may take time but needed for music)
    Serial.println("Init SD...");
    esp_err_t sd_result = SD_Init();
    if (sd_result == ESP_OK) {
        Serial.println("SD ready");
        sd_card_available = true;
    } else {
        Serial.println("SD failed - continuing");
        sd_card_available = false;
    }
    esp_task_wdt_reset(); // Feed watchdog

    // PRIORITY 5: WiFi - non-blocking, continues in background
    Serial.println("Init WiFi...");
    WiFi_Init(); // Returns quickly, connects in background
    esp_task_wdt_reset(); // Feed watchdog
    
    // Create tasks with load balancing across cores
    Serial.println("Starting tasks...");

    // UI task on Core 0 - high priority for responsive display
    xTaskCreatePinnedToCore(
        UI_Task,
        "UITask",
        8192,       // Large stack for LVGL
        NULL,
        2,          // Priority 2 (higher)
        &ui_task_handle,
        0           // Core 0 for UI
    );

    // Driver task on Core 1 - separate from UI to prevent contention
    xTaskCreatePinnedToCore(
        Driver_Loop,
        "DriverTask",
        2048,
        NULL,
        1,          // Priority 1 (lower)
        &driver_task_handle,
        1           // Core 1 - moved from Core 0!
    );

    // Brief delay to let tasks start
    vTaskDelay(pdMS_TO_TICKS(100)); // Reduced from 200ms
    esp_task_wdt_reset(); // Feed watchdog

    // Initialize the UI controller
    UIController_Init();
    esp_task_wdt_reset(); // Feed watchdog

    Serial.println("Boot complete!");
    memory_info();
}

void loop()
{
    // The main loop should be empty since we're using FreeRTOS tasks
    // Just delay to keep the loop task from using CPU
    vTaskDelay(pdMS_TO_TICKS(1000));
}