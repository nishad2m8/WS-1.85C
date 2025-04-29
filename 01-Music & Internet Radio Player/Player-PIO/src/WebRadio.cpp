#include "WebRadio.h"
#include "Audio_PCM5101.h"

int currentStation = 0;
bool radioPlaying = false;
uint32_t streamStartTime = 0;
uint32_t elapsedStreamTime = 0;
uint32_t lastStreamTimeUpdate = 0;
uint32_t streamBitRate = 128; // Default bit rate in kbps

// Initialize the web radio
bool WebRadio_Init() {
    if (!audio_initialized) {
        Serial.println("Audio not initialized, cannot start web radio");
        return false;
    }
    
    // Initialize other radio variables
    currentStation = 0;
    radioPlaying = false;
    streamStartTime = 0;
    elapsedStreamTime = 0;
    
    return true;
}

// Play the current radio station
bool WebRadio_Play() {
    if (!audio_initialized) {
        Serial.println("Audio not initialized, cannot play radio");
        return false;
    }
    
    // Stop any currently playing audio
    audio.stopSong();
    
    // Connect to the radio stream
    if (audio.connecttohost(radioStations[currentStation].url)) {
        Serial.printf("Connecting to %s\n", radioStations[currentStation].name);
        radioPlaying = true;
        streamStartTime = millis();
        lastStreamTimeUpdate = streamStartTime;
        return true;
    } else {
        Serial.println("Connection to radio station failed");
        radioPlaying = false;
        return false;
    }
}

// Forward declaration of internal function
void updateElapsedTime();

// Stop the radio playback
void WebRadio_Stop() {
    if (!audio_initialized) return;
    
    if (radioPlaying) {
        audio.stopSong();
        radioPlaying = false;
        updateElapsedTime();
    }
}

// Toggle play/pause for radio
bool WebRadio_TogglePlayPause() {
    if (!audio_initialized) return false;
    
    if (radioPlaying) {
        WebRadio_Stop();
        return false;
    } else {
        return WebRadio_Play();
    }
}

// Switch to the next station
bool WebRadio_NextStation() {
    if (!audio_initialized) return false;
    
    currentStation = (currentStation + 1) % RADIO_STATION_COUNT;
    
    // If radio was playing, play the new station
    bool wasPlaying = radioPlaying;
    if (wasPlaying) {
        WebRadio_Stop();
        return WebRadio_Play();
    }
    
    return false;
}

// Switch to the previous station
bool WebRadio_PreviousStation() {
    if (!audio_initialized) return false;
    
    currentStation = (currentStation - 1 + RADIO_STATION_COUNT) % RADIO_STATION_COUNT;
    
    // If radio was playing, play the new station
    bool wasPlaying = radioPlaying;
    if (wasPlaying) {
        WebRadio_Stop();
        return WebRadio_Play();
    }
    
    return false;
}

// Get the current station name
const char* WebRadio_GetCurrentStationName() {
    return radioStations[currentStation].name;
}

// Get the current station URL
const char* WebRadio_GetCurrentStationURL() {
    return radioStations[currentStation].url;
}

// Get the current station index
int WebRadio_GetCurrentStationIndex() {
    return currentStation;
}

// Set the current station by index
bool WebRadio_SetStation(int index) {
    if (index < 0 || index >= RADIO_STATION_COUNT) {
        return false;
    }
    
    bool wasPlaying = radioPlaying;
    if (wasPlaying) {
        WebRadio_Stop();
    }
    
    currentStation = index;
    
    if (wasPlaying) {
        return WebRadio_Play();
    }
    
    return true;
}

// Update the elapsed time for the current stream
void updateElapsedTime() {
    if (radioPlaying) {
        uint32_t currentMillis = millis();
        uint32_t delta = currentMillis - lastStreamTimeUpdate;
        elapsedStreamTime += delta;
        lastStreamTimeUpdate = currentMillis;
    }
}

// Get the elapsed time in seconds
uint32_t WebRadio_GetElapsedTime() {
    updateElapsedTime();
    return elapsedStreamTime / 1000;
}

// Reset the elapsed time counter
void WebRadio_ResetElapsedTime() {
    streamStartTime = millis();
    lastStreamTimeUpdate = streamStartTime;
    elapsedStreamTime = 0;
}

// Get radio playing status
bool WebRadio_IsPlaying() {
    return radioPlaying;
}

// Set the bitrate of the current stream (for calculating data rate)
void WebRadio_SetBitrate(uint32_t bitrate) {
    streamBitRate = bitrate;
}

// Get the estimated data rate in kB/s
float WebRadio_GetDataRate() {
    if (radioPlaying) {
        // Simple estimation based on bitrate
        return streamBitRate / 8.0;  // Convert kbps to kB/s
    }
    return 0.0;
}