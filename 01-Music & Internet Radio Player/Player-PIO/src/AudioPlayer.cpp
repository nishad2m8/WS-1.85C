#include "AudioPlayer.h"
#include "Audio_PCM5101.h"
#include "SD_Card.h"
#include "WiFiManager.h"

// Audio player state
static PlayerMode currentMode = MODE_MUSIC_PLAYER;
static int currentTrackIndex = 0;
static int currentStationIndex = 0;
static bool isPlaying = false;
static char mp3FileList[MAX_MP3_FILES][64]; // Reduced buffer size
static int mp3FileCount = 0;
static uint32_t lastPlaybackUpdate = 0;
static TaskHandle_t audioTaskHandle = NULL;

// Volume setting
static uint8_t currentVolume = 10; // Added this variable definition

// Function to free memory before audio playback
void freeMemoryForDecoder() {
    Serial.println("Freeing memory for decoder...");
    
    // Stop current audio
    audio.stopSong();
    
    // Wait for memory to be released
    vTaskDelay(300 / portTICK_PERIOD_MS);
    
    // Force a GC-like operation with smaller allocation
    void* temp[5]; // Reduced from 10 blocks to 5
    for (int i = 0; i < 5; i++) {
        temp[i] = malloc(256); // Reduced allocation size
    }
    for (int i = 0; i < 5; i++) {
        if (temp[i]) free(temp[i]);
    }
    
    // Print memory info
    Serial.printf("Heap after optimization: %d bytes\n", esp_get_free_heap_size());
}

// Initialize the audio player
bool AudioPlayer_Init() {
    Serial.println("Initializing audio player with minimal memory usage...");
    
    // Setup audio
    if (!audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
        Serial.println("Audio hardware initialization failed");
        return false;
    }
    
    // Set volume to medium
    audio.setVolume(currentVolume);
    
    // Create dedicated audio task with smaller stack
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        AudioPlayer_Task,
        "AudioTask",
        3584, // Increased from 3072 (needed for audio processing)
        NULL,
        3,
        &audioTaskHandle,
        1
    );
    
    if (taskCreated != pdPASS) {
        Serial.println("Failed to create audio task");
        return false;
    }
    
    // Scan audio files only if SD card is available
    if (SD_IsAvailable()) {
        mp3FileCount = AudioPlayer_ScanMP3Files();
        Serial.printf("Found %d valid audio files on SD card\n", mp3FileCount);
    }
    
    Serial.println("Audio player initialized successfully");
    return true;
}

// Audio task to process audio data - runs on Core 1
void AudioPlayer_Task(void *parameter) {
    Serial.println("Audio task started on Core 1");
    static bool wasPlaying = false;
    
    while (true) {
        // Only process when running
        if (audio.isRunning()) {
            audio.loop();
            wasPlaying = true;
        } else if (wasPlaying && isPlaying) {
            // Track just finished playing (was playing but audio stopped while isPlaying is still true)
            Serial.println("Audio playback ended");
            
            if (currentMode == MODE_MUSIC_PLAYER) {
                // Auto-advance to next track only in music player mode
                Serial.println("Auto-advancing to next track");
                
                // Mark current track as stopped
                isPlaying = false;
                
                // Calculate next track index with loop
                int nextTrack = (currentTrackIndex + 1) % mp3FileCount;
                
                // Set next track and play it
                AudioPlayer_SetTrack(nextTrack);
                AudioPlayer_Play();
            } else {
                // For web radio, just mark as stopped
                isPlaying = false;
            }
            wasPlaying = false;
        }
        
        // Task period needs to be fast enough for audio decoding
        vTaskDelay(10); // Lower delay for smoother audio processing
    }
}

// Set the current player mode
void AudioPlayer_SetMode(PlayerMode mode) {
    // Stop any current playback first
    AudioPlayer_Stop();
    vTaskDelay(200);
    
    currentMode = mode;
    Serial.printf("Player mode changed to: %s\n", 
                 mode == MODE_MUSIC_PLAYER ? "Music Player" : "Internet Radio");
}

// Get the current player mode
PlayerMode AudioPlayer_GetMode() {
    return currentMode;
}

// Play current track or station
bool AudioPlayer_Play() {
    // Stop current playback and free resources
    AudioPlayer_Stop();
    vTaskDelay(300 / portTICK_PERIOD_MS);
    
    // Free memory
    freeMemoryForDecoder();
    
    if (currentMode == MODE_MUSIC_PLAYER) {
        return AudioPlayer_PlayTrack(currentTrackIndex);
    } else {
        return AudioPlayer_PlayStation(currentStationIndex);
    }
}

// Stop playback
void AudioPlayer_Stop() {
    if (isPlaying) {
        audio.stopSong();
        isPlaying = false;
        Serial.println("Playback stopped");
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// Toggle play/pause
bool AudioPlayer_TogglePlayPause() {
    if (isPlaying) {
        AudioPlayer_Stop();
        return false;
    } else {
        return AudioPlayer_Play();
    }
}

// Go to next track or station
bool AudioPlayer_Next() {
    if (currentMode == MODE_MUSIC_PLAYER) {
        if (mp3FileCount > 0) {
            currentTrackIndex = (currentTrackIndex + 1) % mp3FileCount;
            Serial.printf("Next track: %d - %s\n", currentTrackIndex, mp3FileList[currentTrackIndex]);
            return true;
        }
    } else {
        currentStationIndex = (currentStationIndex + 1) % RADIO_STATION_COUNT;
        Serial.printf("Next station: %d - %s\n", currentStationIndex, radioStations[currentStationIndex].name);
        return true;
    }
    return false;
}

// Go to previous track or station
bool AudioPlayer_Previous() {
    if (currentMode == MODE_MUSIC_PLAYER) {
        if (mp3FileCount > 0) {
            currentTrackIndex = (currentTrackIndex - 1 + mp3FileCount) % mp3FileCount;
            Serial.printf("Previous track: %d - %s\n", currentTrackIndex, mp3FileList[currentTrackIndex]);
            return true;
        }
    } else {
        currentStationIndex = (currentStationIndex - 1 + RADIO_STATION_COUNT) % RADIO_STATION_COUNT;
        Serial.printf("Previous station: %d - %s\n", currentStationIndex, radioStations[currentStationIndex].name);
        return true;
    }
    return false;
}

// Play a specific MP3 track from SD card - with minimal memory usage
bool AudioPlayer_PlayTrack(int index) {
    if (!SD_IsAvailable() || mp3FileCount == 0) {
        Serial.println("Cannot play track: No audio files or SD card unavailable");
        return false;
    }
    
    // Validate index
    if (index < 0 || index >= mp3FileCount) {
        Serial.printf("Invalid track index: %d (max: %d)\n", index, mp3FileCount - 1);
        return false;
    }
    
    currentTrackIndex = index;
    
    // Build file path
    const char* directory = "/";
    const char* fileName = mp3FileList[currentTrackIndex];
    
    // Check if file exists
    Serial.printf("Checking if file '%s%s' exists...\n", directory, fileName);
    if (!SD_IsAvailable() || !File_Search(directory, fileName)) {
        Serial.printf("Audio file not found: %s\n", fileName);
        return false;
    }
    
    Serial.printf("File '%s%s' found in root directory.\n", directory, fileName);
    
    // Build full path with reduced buffer size
    const int maxPathLength = 80;
    char filePath[maxPathLength];
    if (strcmp(directory, "/") == 0) {                                               
        snprintf(filePath, maxPathLength, "%s%s", directory, fileName);   
    } else {                                                            
        snprintf(filePath, maxPathLength, "%s/%s", directory, fileName);
    }
    
    Serial.printf("Playing file: %s\n", filePath);
    Serial.printf("Free heap before playback: %d bytes\n", esp_get_free_heap_size());
    
    // Try simplified approach with memory handling
    try {
        // Call stopSong first to ensure clean start
        audio.stopSong();
        vTaskDelay(50);
        
        // Explicit connection to SD with proper file path
        bool ret = audio.connecttoFS(SD_MMC, filePath);
        if (ret) {
            Serial.println("Audio file loaded successfully!");
            isPlaying = true;
            
            // Set volume again after connection
            audio.setVolume(currentVolume);
            Serial.println("Play");
            return true;
        } else {
            Serial.println("Failed to load audio file");
            isPlaying = false;
            return false;
        }
    } catch (...) {
        Serial.println("Exception during playback attempt");
        isPlaying = false;
        return false;
    }
}

// Play a specific internet radio station
bool AudioPlayer_PlayStation(int index) {
    // Check WiFi connection
    if (!WiFi_IsConnected()) {
        Serial.println("WiFi not connected, cannot play internet radio");
        return false;
    }
    
    // Validate index
    if (index < 0 || index >= RADIO_STATION_COUNT) {
        Serial.printf("Invalid station index: %d (max: %d)\n", index, RADIO_STATION_COUNT - 1);
        return false;
    }
    
    currentStationIndex = index;
    
    // Complete stop of any current playback
    AudioPlayer_Stop();
    vTaskDelay(500 / portTICK_PERIOD_MS); // Longer wait for radio
    
    Serial.printf("Connecting to station: %s\n", radioStations[currentStationIndex].name);
    
    // Free extra memory before connecting
    freeMemoryForDecoder();
    
    // Set radio-specific volume (lower to prevent distortion)
    audio.setVolume(15); // Reduced from 18
    
    // Connect to the radio stream with timeout handling
    bool ret = false;
    
    // Try to connect with timeout protection
    try {
        // Explicitly call stopSong first
        audio.stopSong();
        vTaskDelay(100);
        
        // Connect to stream
        ret = audio.connecttohost(radioStations[currentStationIndex].url);
        
        // Wait for connection to establish
        if (ret) {
            Serial.printf("Connected to radio station: %s\n", radioStations[currentStationIndex].name);
            isPlaying = true;
            return true;
        } else {
            Serial.println("Connection to radio station failed");
            isPlaying = false;
            return false;
        }
    } catch (...) {
        Serial.println("Exception during radio connection");
        isPlaying = false;
        return false;
    }
}

// Scan SD card for MP3 files only (no hidden files, no macOS metadata)
int AudioPlayer_ScanMP3Files() {
    if (!SD_IsAvailable()) {
        Serial.println("SD card not available for scanning audio files");
        return 0;
    }
    
    // Clear existing files list
    memset(mp3FileList, 0, sizeof(mp3FileList));
    
    Serial.println("Starting SD card scan for audio files...");
    
    // Using direct file scanning (more memory efficient)
    File root = SD_MMC.open("/");
    if (root && root.isDirectory()) {
        int fileIndex = 0;
        
        // Scan for MP3 files
        File file = root.openNextFile();
        while (file && fileIndex < MAX_MP3_FILES) {
            if (!file.isDirectory()) {
                String fileName = file.name();
                
                // Only include normal MP3 files (no metadata or hidden files)
                if ((fileName.endsWith(".mp3") || fileName.endsWith(".MP3")) && 
                    !fileName.startsWith("._") && !fileName.startsWith(".")) {
                    
                    Serial.printf("Valid MP3: %s (%d bytes)\n", file.name(), file.size());
                    
                    // Copy filename with length check
                    strncpy(mp3FileList[fileIndex], file.name(), 63);
                    mp3FileList[fileIndex][63] = '\0'; // Ensure null termination
                    fileIndex++;
                    
                    // Release some CPU time after each file
                    if (fileIndex % 2 == 0) { // More frequent yields
                        vTaskDelay(1);
                    }
                }
            }
            file = root.openNextFile();
        }
        
        file.close();
        root.close();
        
        mp3FileCount = fileIndex;
        Serial.printf("Found %d valid MP3 files\n", mp3FileCount);
    } else {
        Serial.println("Failed to open root directory");
    }
    
    return mp3FileCount;
}

// Get playing status
bool AudioPlayer_IsPlaying() {
    return isPlaying && audio.isRunning();
}

// Get total duration of current track (applicable only in music mode)
uint32_t AudioPlayer_GetTotalDuration() {
    if (currentMode != MODE_MUSIC_PLAYER) {
        return 0;
    }
    return audio.getAudioFileDuration();
}

// Get elapsed time
uint32_t AudioPlayer_GetElapsedTime() {
    if (currentMode == MODE_MUSIC_PLAYER) {
        return audio.getAudioCurrentTime();
    } else {
        // For radio, just return a fixed value
        static uint32_t radioTime = 0;
        if (isPlaying) {
            radioTime += 1;
        }
        return radioTime;
    }
}

// Get track/station progress (0.0 to 1.0) - only meaningful for music
float AudioPlayer_GetProgress() {
    if (currentMode != MODE_MUSIC_PLAYER) {
        return 0.0f;
    }
    
    uint32_t total = AudioPlayer_GetTotalDuration();
    if (total == 0) return 0.0f;
    
    uint32_t elapsed = AudioPlayer_GetElapsedTime();
    return (float)elapsed / (float)total;
}

// Get number of audio files
int AudioPlayer_GetFileCount() {
    return mp3FileCount;
}

// Get the name of the current MP3 file or radio station
const char* AudioPlayer_GetCurrentName() {
    if (currentMode == MODE_MUSIC_PLAYER) {
        if (mp3FileCount > 0) {
            return mp3FileList[currentTrackIndex];
        } else {
            return "No audio files";
        }
    } else {
        return radioStations[currentStationIndex].name;
    }
}

// Get audio file name by index
const char* AudioPlayer_GetFileName(int index) {
    if (index < 0 || index >= mp3FileCount) {
        return "Invalid index";
    }
    return mp3FileList[index];
}

// Get radio station name by index
const char* AudioPlayer_GetStationName(int index) {
    if (index < 0 || index >= RADIO_STATION_COUNT) {
        return "Invalid index";
    }
    return radioStations[index].name;
}

// Get current track index
int AudioPlayer_GetCurrentTrackIndex() {
    return currentTrackIndex;
}

// Get current station index
int AudioPlayer_GetCurrentStationIndex() {
    return currentStationIndex;
}

// Set track by index
bool AudioPlayer_SetTrack(int index) {
    if (index < 0 || index >= mp3FileCount) {
        return false;
    }
    
    currentTrackIndex = index;
    return true;
}

// Set station by index
bool AudioPlayer_SetStation(int index) {
    if (index < 0 || index >= RADIO_STATION_COUNT) {
        return false;
    }
    
    currentStationIndex = index;
    return true;
}

// Set the volume level (0-21)
void AudioPlayer_SetVolume(uint8_t volume) {
    if (volume > 21) volume = 21;
    currentVolume = volume;
    audio.setVolume(currentVolume);
}

// Get the current volume level
uint8_t AudioPlayer_GetVolume() {
    return currentVolume;
}