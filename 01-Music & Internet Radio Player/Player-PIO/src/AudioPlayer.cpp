#include "AudioPlayer.h"
#include "config.h"

#ifdef HARDWARE_V2
#include "Audio_ES8311.h"
#else
#include "Audio_PCM5101.h"
#endif

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

// Function to free memory before audio playback (non-blocking)
void freeMemoryForDecoder() {
    audio.stopSong();
}

// Initialize the audio player
bool AudioPlayer_Init() {
    // Setup audio
    if (!audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT)) {
        return false;
    }

    audio.setVolume(currentVolume);

    // Create dedicated audio task on Core 1 (increased stack for complex streams)
    BaseType_t taskCreated = xTaskCreatePinnedToCore(
        AudioPlayer_Task,
        "AudioTask",
        8192,       // Increased from 4096 to handle complex radio streams
        NULL,
        3,
        &audioTaskHandle,
        1
    );

    if (taskCreated != pdPASS) {
        return false;
    }

    // Scan audio files if SD card available
    if (SD_IsAvailable()) {
        mp3FileCount = AudioPlayer_ScanMP3Files();
    }

    return true;
}

// Audio task to process audio data - runs on Core 1
void AudioPlayer_Task(void *parameter) {
    static bool wasPlaying = false;

    while (true) {
        if (audio.isRunning()) {
            audio.loop();
            wasPlaying = true;
        } else if (wasPlaying && isPlaying) {
            // Track finished playing
            if (currentMode == MODE_MUSIC_PLAYER && mp3FileCount > 0) {
                // Auto-advance to next track
                isPlaying = false;
                int nextTrack = (currentTrackIndex + 1) % mp3FileCount;
                AudioPlayer_SetTrack(nextTrack);
                AudioPlayer_Play();
            } else {
                isPlaying = false;
            }
            wasPlaying = false;
        }

        vTaskDelay(5); // Fast loop for smooth audio
    }
}

// Set the current player mode
void AudioPlayer_SetMode(PlayerMode mode) {
    // Stop any current playback first
    AudioPlayer_Stop();
    currentMode = mode;
}

// Get the current player mode
PlayerMode AudioPlayer_GetMode() {
    return currentMode;
}

// Play current track or station
bool AudioPlayer_Play() {
    // Stop current playback
    audio.stopSong();
    isPlaying = false;

    if (currentMode == MODE_MUSIC_PLAYER) {
        return AudioPlayer_PlayTrack(currentTrackIndex);
    } else {
        return AudioPlayer_PlayStation(currentStationIndex);
    }
}

// Stop playback
void AudioPlayer_Stop() {
    audio.stopSong();
    isPlaying = false;
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
            return true;
        }
    } else {
        currentStationIndex = (currentStationIndex + 1) % RADIO_STATION_COUNT;
        return true;
    }
    return false;
}

// Go to previous track or station
bool AudioPlayer_Previous() {
    if (currentMode == MODE_MUSIC_PLAYER) {
        if (mp3FileCount > 0) {
            currentTrackIndex = (currentTrackIndex - 1 + mp3FileCount) % mp3FileCount;
            return true;
        }
    } else {
        currentStationIndex = (currentStationIndex - 1 + RADIO_STATION_COUNT) % RADIO_STATION_COUNT;
        return true;
    }
    return false;
}

// Play a specific MP3 track from SD card
bool AudioPlayer_PlayTrack(int index) {
    if (!SD_IsAvailable() || mp3FileCount == 0) {
        return false;
    }

    // Validate index
    if (index < 0 || index >= mp3FileCount) {
        return false;
    }

    currentTrackIndex = index;

    // Build file path
    char filePath[80];
    snprintf(filePath, sizeof(filePath), "/%s", mp3FileList[currentTrackIndex]);

    // Stop current and connect to file
    audio.stopSong();
    bool ret = audio.connecttoFS(SD_MMC, filePath);

    if (ret) {
        isPlaying = true;
        audio.setVolume(currentVolume);
    } else {
        isPlaying = false;
    }

    return ret;
}

// Play a specific internet radio station
bool AudioPlayer_PlayStation(int index) {
    // Check WiFi connection
    if (!WiFi_IsConnected()) {
        return false;
    }

    // Validate index
    if (index < 0 || index >= RADIO_STATION_COUNT) {
        return false;
    }

    currentStationIndex = index;

    // Stop current and connect to stream
    audio.stopSong();
    audio.setVolume(currentVolume);

    bool ret = audio.connecttohost(radioStations[currentStationIndex].url);
    isPlaying = ret;
    return ret;
}

// Scan SD card for MP3 files only (no hidden files, no macOS metadata)
int AudioPlayer_ScanMP3Files() {
    if (!SD_IsAvailable()) {
        return 0;
    }

    memset(mp3FileList, 0, sizeof(mp3FileList));

    File root = SD_MMC.open("/");
    if (root && root.isDirectory()) {
        int fileIndex = 0;
        File file = root.openNextFile();

        while (file && fileIndex < MAX_MP3_FILES) {
            if (!file.isDirectory()) {
                String fileName = file.name();

                // Only include normal MP3 files (no metadata or hidden files)
                if ((fileName.endsWith(".mp3") || fileName.endsWith(".MP3")) &&
                    !fileName.startsWith("._") && !fileName.startsWith(".")) {

                    strncpy(mp3FileList[fileIndex], file.name(), 63);
                    mp3FileList[fileIndex][63] = '\0';
                    fileIndex++;
                }
            }
            file = root.openNextFile();
        }

        file.close();
        root.close();
        mp3FileCount = fileIndex;
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

#ifdef HARDWARE_V2
    // Also set ES8311 codec volume (convert 0-21 to 0-100 scale)
    int codec_volume = (currentVolume * 100) / 21;
    ES8311_SetVolume(codec_volume);
#endif
}

// Get the current volume level
uint8_t AudioPlayer_GetVolume() {
    return currentVolume;
}

// Get buffer fill percentage (0-100) for streaming
int AudioPlayer_GetBufferFillPercent() {
    uint32_t bufferSize = audio.inBufferSize();
    if (bufferSize == 0) return 0;

    uint32_t bufferFilled = audio.inBufferFilled();
    int percent = (bufferFilled * 100) / bufferSize;

    if (percent > 100) percent = 100;
    if (percent < 0) percent = 0;

    return percent;
}

// Get bitrate in kbps
uint32_t AudioPlayer_GetBitRate() {
    return audio.getBitRate() / 1000; // Convert to kbps
}