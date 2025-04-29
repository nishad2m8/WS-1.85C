#pragma once
#include "Arduino.h"
#include "RadioStations.h"

// Reduce max files to handle to save memory
#define MAX_MP3_FILES 50  // Reduced from 100

// Player mode
typedef enum {
    MODE_MUSIC_PLAYER = 0,
    MODE_WEB_RADIO = 1
} PlayerMode;

// Function to initialize the audio player
bool AudioPlayer_Init();

// Audio task function (runs on Core 1)
void AudioPlayer_Task(void *parameter);

// Mode control
void AudioPlayer_SetMode(PlayerMode mode);
PlayerMode AudioPlayer_GetMode();

// Playback control
bool AudioPlayer_Play();
void AudioPlayer_Stop();
bool AudioPlayer_TogglePlayPause();
bool AudioPlayer_Next();
bool AudioPlayer_Previous();

// Specific playback commands
bool AudioPlayer_PlayTrack(int index);
bool AudioPlayer_PlayStation(int index);

// Audio file management
int AudioPlayer_ScanMP3Files();

// Status functions
bool AudioPlayer_IsPlaying();
uint32_t AudioPlayer_GetTotalDuration();
uint32_t AudioPlayer_GetElapsedTime();
float AudioPlayer_GetProgress();
int AudioPlayer_GetFileCount();
const char* AudioPlayer_GetCurrentName();
const char* AudioPlayer_GetFileName(int index);
const char* AudioPlayer_GetStationName(int index);
int AudioPlayer_GetCurrentTrackIndex();
int AudioPlayer_GetCurrentStationIndex();

// Selection functions
bool AudioPlayer_SetTrack(int index);
bool AudioPlayer_SetStation(int index);

// Volume control functions
void AudioPlayer_SetVolume(uint8_t volume);
uint8_t AudioPlayer_GetVolume();