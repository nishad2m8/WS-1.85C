#include "UIController.h"
#include "Audio_PCM5101.h"
#include "Display_ST77916.h"
#include "AudioPlayer.h" 

// Maximum number of files and stations to display
const int MAX_FILES = 20;     // Increased to 10
const int MAX_STATIONS = 10;  // Increased to 10

// Timer for UI updates
static lv_timer_t *ui_update_timer = NULL;

// Static buffer to hold display text
static char timeDisplayBuffer[24];

// Last update time for UI refresh optimization
static uint32_t lastProgressUpdate = 0;
static uint32_t lastTimeUpdate = 0;

// Brightness setting
static int currentBrightness = 40;

// Static buffer for list display - allocate statically to avoid stack issues
static char listBuffer[1536]; // Increased size but still reasonable

// To track radio connection errors
static bool radioConnectionError = false;

// Error display duration
static uint32_t errorDisplayStartTime = 0;
const uint32_t ERROR_DISPLAY_DURATION = 3000; // 3 seconds

// Forward declaration of timer callback
static void UIController_TimerCallback(lv_timer_t *timer);

// Initialize the UI controller
void UIController_Init() {
    // Set initial volume using audio object directly
    AudioPlayer_SetVolume(10);
    
    // Set initial brightness
    Set_Backlight(currentBrightness);
    
    // Initialize the audio player with smaller buffers
    if (AudioPlayer_Init()) {
        Serial.println("Audio player initialized successfully");
        
        // Delay list update to separate initialization phases
        vTaskDelay(20);
        UIController_UpdateList();
    } else {
        Serial.println("Audio player initialization failed");
    }
    
    // Set up event handlers for UI elements
    UIController_SetupEvents();
    
    // Create timer for UI updates with reduced frequency
    ui_update_timer = lv_timer_create(UIController_TimerCallback, 200, NULL);
    
    // Initial UI update
    UIController_UpdateUI();
    
    // Make sure playback is paused on startup
    AudioPlayer_Stop();
    lv_obj_clear_state(ui_Button_PlayPause, LV_STATE_CHECKED);
    
    // Initialize volume and brightness arc controls
    lv_arc_set_value(ui_Arc_Volume, AudioPlayer_GetVolume() * 100 / 21);
    lv_arc_set_value(ui_Arc_Brightness, currentBrightness);
}

// Timer callback for UI updates
static void UIController_TimerCallback(lv_timer_t *timer) {
    UIController_UpdateUI();
}

// Process UI events in a loop (called from main loop)
void UIController_Loop() {
    // Empty function - UI updates handled by timer
}

// Update UI elements with current state
void UIController_UpdateUI() {
    uint32_t currentMillis = millis();
    
    // Check if we need to update the roller to match current track
    static int lastTrackIndex = -1;
    static int lastStationIndex = -1;
    
    int currentTrackIndex = AudioPlayer_GetCurrentTrackIndex();
    int currentStationIndex = AudioPlayer_GetCurrentStationIndex();
    
    // If track/station index has changed, update the roller
    if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER && currentTrackIndex != lastTrackIndex) {
        lv_roller_set_selected(ui_Roller_list, currentTrackIndex, LV_ANIM_OFF);
        lastTrackIndex = currentTrackIndex;
    } else if (AudioPlayer_GetMode() == MODE_WEB_RADIO && currentStationIndex != lastStationIndex) {
        lv_roller_set_selected(ui_Roller_list, currentStationIndex, LV_ANIM_OFF);
        lastStationIndex = currentStationIndex;
    }
    
    // Update time display every 1 second
    if (currentMillis - lastTimeUpdate >= 1000) {
        UIController_UpdateTimeDisplay();
        lastTimeUpdate = currentMillis;
    }
    
    // Update progress bar every 500ms
    if (currentMillis - lastProgressUpdate >= 500) {
        UIController_UpdateProgressBar();
        lastProgressUpdate = currentMillis;
    }
    
    // Update play/pause button state
    lv_obj_add_state(ui_Button_PlayPause, AudioPlayer_IsPlaying() ? LV_STATE_CHECKED : 0);
    
    // Update mode button state
    lv_obj_add_state(ui_Button_Mode, AudioPlayer_GetMode() == MODE_WEB_RADIO ? LV_STATE_CHECKED : 0);
}

// Set up event handlers for UI elements
void UIController_SetupEvents() {
    // Mode button
    lv_obj_add_event_cb(ui_Button_Mode, UI_ModeButtonCallback, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Play/Pause button
    lv_obj_add_event_cb(ui_Button_PlayPause, UI_PlayPauseButtonCallback, LV_EVENT_CLICKED, NULL);
    
    // Next button
    lv_obj_add_event_cb(ui_Button_Next, UI_NextButtonCallback, LV_EVENT_CLICKED, NULL);
    
    // Previous button
    lv_obj_add_event_cb(ui_Button_Previous, UI_PreviousButtonCallback, LV_EVENT_CLICKED, NULL);
    
    // List roller
    lv_obj_add_event_cb(ui_Roller_list, UI_ListCallback, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Volume arc control
    lv_obj_add_event_cb(ui_Arc_Volume, UI_VolumeArcCallback, LV_EVENT_VALUE_CHANGED, NULL);
    
    // Brightness arc control
    lv_obj_add_event_cb(ui_Arc_Brightness, UI_BrightnessArcCallback, LV_EVENT_VALUE_CHANGED, NULL);
}

// Update the time display - Now with error display
void UIController_UpdateTimeDisplay() {
    if (radioConnectionError && AudioPlayer_GetMode() == MODE_WEB_RADIO) {
        // Show error message for radio
        lv_label_set_text(ui_Label_Time, "Error !!!");
        // Don't reset - let other UI actions clear the error
    } else if (!AudioPlayer_IsPlaying()) {
        lv_label_set_text(ui_Label_Time, "--:--");
    } else {
        uint32_t elapsed;
        if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
            // For MP3, show remaining time
            uint32_t total = AudioPlayer_GetTotalDuration();
            elapsed = AudioPlayer_GetElapsedTime();
            uint32_t remaining = (total > elapsed) ? (total - elapsed) : 0;
            
            // Just show minutes and seconds
            snprintf(timeDisplayBuffer, sizeof(timeDisplayBuffer), "-%02d:%02d", 
                    remaining / 60, remaining % 60);
        } else {
            // For radio, show elapsed time
            elapsed = AudioPlayer_GetElapsedTime();
            snprintf(timeDisplayBuffer, sizeof(timeDisplayBuffer), "%02d:%02d", 
                    elapsed / 60, elapsed % 60);
        }
        
        // Update the label
        lv_label_set_text(ui_Label_Time, timeDisplayBuffer);
    }
}

// Update the progress bar
void UIController_UpdateProgressBar() {
    if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
        if (AudioPlayer_IsPlaying()) {
            int progress = (int)(AudioPlayer_GetProgress() * 100);
            lv_bar_set_value(ui_Bar1, progress, LV_ANIM_OFF);
        } else {
            lv_bar_set_value(ui_Bar1, 0, LV_ANIM_OFF);
        }
    } else {
        // Web radio mode - simplified indicator
        static uint8_t radioIndicator = 50;
        lv_bar_set_value(ui_Bar1, AudioPlayer_IsPlaying() ? radioIndicator : 0, LV_ANIM_OFF);
    }
}

// Update the file/station list - IMPROVED VERSION
void UIController_UpdateList() {
    Serial.printf("Updating UI list, free memory: %d bytes\n", esp_get_free_heap_size());
    
    // Clear buffer
    memset(listBuffer, 0, sizeof(listBuffer));
    
    if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
        // Get MP3 file list
        int fileCount = AudioPlayer_GetFileCount();
        Serial.printf("Updating UI list with %d files\n", fileCount);
        
        if (fileCount > 0) {
            // Calculate how many files to show
            int filesToShow = (fileCount > MAX_FILES) ? MAX_FILES : fileCount;
            
            // Create a temporary small buffer for each filename
            char tempName[32]; // Temp buffer for truncated filename
            
            // Build the list with actual filenames (truncated if needed)
            for (int i = 0; i < filesToShow; i++) {
                const char* fileName = AudioPlayer_GetFileName(i);
                
                // Get filename without extension
                strncpy(tempName, fileName, sizeof(tempName) - 1);
                tempName[sizeof(tempName) - 1] = '\0';
                
                // Truncate long filenames
                if (strlen(tempName) > 20) {
                    tempName[17] = '.';
                    tempName[18] = '.';
                    tempName[19] = '.';
                    tempName[20] = '\0';
                }
                
                // Add to list buffer
                strcat(listBuffer, tempName);
                
                // Add newline between items
                if (i < filesToShow - 1) {
                    strcat(listBuffer, "\n");
                }
                
                // Give system time to process
                if (i % 2 == 0) {
                    vTaskDelay(1);
                }
            }
            
            // Set the options
            lv_roller_set_options(ui_Roller_list, listBuffer, LV_ROLLER_MODE_NORMAL);
            
            // Set selected item
            int currentIndex = AudioPlayer_GetCurrentTrackIndex();
            if (currentIndex < filesToShow) {
                lv_roller_set_selected(ui_Roller_list, currentIndex, LV_ANIM_OFF);
            } else {
                lv_roller_set_selected(ui_Roller_list, 0, LV_ANIM_OFF);
            }
        } else {
            // No files found
            strcpy(listBuffer, "No MP3 files");
            lv_roller_set_options(ui_Roller_list, listBuffer, LV_ROLLER_MODE_NORMAL);
        }
    } else {
        // Show radio stations
        int stationCount = RADIO_STATION_COUNT;
        int stationsToShow = (stationCount > MAX_STATIONS) ? MAX_STATIONS : stationCount;
        
        // Build station list
        for (int i = 0; i < stationsToShow; i++) {
            const char* stationName = AudioPlayer_GetStationName(i);
            
            // Add station name (limited to 20 chars)
            if (strlen(stationName) > 20) {
                strncat(listBuffer, stationName, 17);
                strcat(listBuffer, "...");
            } else {
                strcat(listBuffer, stationName);
            }
            
            // Add newline between items
            if (i < stationsToShow - 1) {
                strcat(listBuffer, "\n");
            }
            
            // Give system time to process
            vTaskDelay(1);
        }
        
        // Set the options
        lv_roller_set_options(ui_Roller_list, listBuffer, LV_ROLLER_MODE_NORMAL);
        
        // Set selected item
        int currentStation = AudioPlayer_GetCurrentStationIndex();
        if (currentStation < stationsToShow) {
            lv_roller_set_selected(ui_Roller_list, currentStation, LV_ANIM_OFF);
        } else {
            lv_roller_set_selected(ui_Roller_list, 0, LV_ANIM_OFF);
        }
    }
    
    Serial.printf("List update complete, free memory: %d bytes\n", esp_get_free_heap_size());
}

// UI event callbacks - FORCED AUTO-PLAY FOR NAVIGATION

void UI_ModeButtonCallback(lv_event_t *e) {
    lv_obj_t *obj = lv_event_get_target(e);
    bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
    
    // Stop any current playback first
    AudioPlayer_Stop();
    lv_obj_clear_state(ui_Button_PlayPause, LV_STATE_CHECKED);
    
    // Clear any error when changing modes
    radioConnectionError = false;
    
    // Set mode based on checked state
    AudioPlayer_SetMode(checked ? MODE_WEB_RADIO : MODE_MUSIC_PLAYER);
    
    // Update the list with delay
    vTaskDelay(100);
    UIController_UpdateList();
}

void UI_PlayPauseButtonCallback(lv_event_t *e) {
    // Toggle playback - only plays when explicitly pressed
    bool isPlaying = AudioPlayer_TogglePlayPause();
    
    // Check for radio error or clear existing error
    if (AudioPlayer_GetMode() == MODE_WEB_RADIO) {
        if (isPlaying) {
            // Successfully connected to radio, clear error
            radioConnectionError = false;
        } else {
            // Failed to connect, show error
            radioConnectionError = true;
        }
    }
    
    // Update button state
    lv_obj_add_state(ui_Button_PlayPause, isPlaying ? LV_STATE_CHECKED : 0);
}

// WITH FORCED AUTO-PLAY: Next button definitely plays the next track
void UI_NextButtonCallback(lv_event_t *e) {
    bool wasPlaying = AudioPlayer_IsPlaying();
    
    // Go to next track or station without stopping first
    if (AudioPlayer_Next()) {
        // Update the selected item in the list
        if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
            lv_roller_set_selected(ui_Roller_list, AudioPlayer_GetCurrentTrackIndex(), LV_ANIM_OFF);
        } else {
            lv_roller_set_selected(ui_Roller_list, AudioPlayer_GetCurrentStationIndex(), LV_ANIM_OFF);
        }
        
        // Force auto-play if in music player mode or was previously playing in radio mode
        if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER || wasPlaying) {
            Serial.println("UI: Auto-playing after Next button");
            bool success = AudioPlayer_Play();
            
            // Check for radio error
            if (AudioPlayer_GetMode() == MODE_WEB_RADIO && !success) {
                radioConnectionError = true;
                UIController_UpdateTimeDisplay(); // Force update to show error
            }
            
            // Update UI button state
            lv_obj_add_state(ui_Button_PlayPause, success ? LV_STATE_CHECKED : 0);
        }
    }
}

// WITH FORCED AUTO-PLAY: Previous button definitely plays the previous track
void UI_PreviousButtonCallback(lv_event_t *e) {
    // Stop current playback first
    AudioPlayer_Stop();
    
    // Go to previous track or station
    if (AudioPlayer_Previous()) {
        // Update the selected item in the list
        if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
            lv_roller_set_selected(ui_Roller_list, AudioPlayer_GetCurrentTrackIndex(), LV_ANIM_OFF);
        } else {
            lv_roller_set_selected(ui_Roller_list, AudioPlayer_GetCurrentStationIndex(), LV_ANIM_OFF);
        }
        
        // Force auto-play of the previous item
        Serial.println("UI: Auto-playing after Previous button");
        bool success = AudioPlayer_Play();
        
        // Check for radio error
        if (AudioPlayer_GetMode() == MODE_WEB_RADIO && !success) {
            radioConnectionError = true;
            UIController_UpdateTimeDisplay(); // Force update to show error
        }
        
        // Update UI button state
        lv_obj_add_state(ui_Button_PlayPause, success ? LV_STATE_CHECKED : 0);
    }
}

// List selection doesn't auto-play
void UI_ListCallback(lv_event_t *e) {
    // Get selected index
    int selectedIndex = lv_roller_get_selected(ui_Roller_list);
    
    // Stop any current playback
    AudioPlayer_Stop();
    lv_obj_clear_state(ui_Button_PlayPause, LV_STATE_CHECKED);
    
    // Set the correct track or station
    if (AudioPlayer_GetMode() == MODE_MUSIC_PLAYER) {
        AudioPlayer_SetTrack(selectedIndex);
    } else {
        AudioPlayer_SetStation(selectedIndex);
    }
}

/* 
// Volume control callback
void UI_VolumeArcCallback(lv_event_t *e) {
    lv_obj_t *arc = lv_event_get_target(e);
    int volume_percent = lv_arc_get_value(arc);
    
    // Convert 0-100 to 0-21 for audio library
    uint8_t volume = (volume_percent * 21) / 100;
    
    // Set volume using AudioPlayer function
    AudioPlayer_SetVolume(volume);
}
*/

// With reverse mode fix
void UI_VolumeArcCallback(lv_event_t *e) {
    lv_obj_t *arc = lv_event_get_target(e);
    int value = lv_arc_get_value(arc); // Value from 0 to 100

    // Define background arc angles
    int bg_start = 310;
    int bg_end = 380;
    int angle_span = bg_end - bg_start;

    // Calculate proportion
    float proportion = (float)value / 100.0f;

    // Calculate indicator angles
    int end_angle = bg_end;
    int start_angle = bg_end - (int)(angle_span * proportion);

    // Set indicator angles manually
    lv_arc_set_angles(arc, start_angle, end_angle);

    // Convert value to volume scale (0-21)
    uint8_t volume = (value * 21) / 100;
    AudioPlayer_SetVolume(volume);
}


// Brightness control callback
void UI_BrightnessArcCallback(lv_event_t *e) {
    lv_obj_t *arc = lv_event_get_target(e);
    currentBrightness = lv_arc_get_value(arc);
    
    // Ensure brightness is within valid range (10-100)
    if (currentBrightness < 10) currentBrightness = 10;
    
    // Set backlight brightness
    Set_Backlight(currentBrightness);
}