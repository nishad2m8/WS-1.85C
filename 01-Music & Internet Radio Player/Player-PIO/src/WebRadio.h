#pragma once
#include "Arduino.h"
#include "Audio.h"
#include "RadioStations.h" // Use the common RadioStations header

// Function declarations
bool WebRadio_Init();
bool WebRadio_Play();
void WebRadio_Stop();
bool WebRadio_TogglePlayPause();
bool WebRadio_NextStation();
bool WebRadio_PreviousStation();
const char* WebRadio_GetCurrentStationName();
const char* WebRadio_GetCurrentStationURL();
int WebRadio_GetCurrentStationIndex();
bool WebRadio_SetStation(int index);
uint32_t WebRadio_GetElapsedTime();
void WebRadio_ResetElapsedTime();
bool WebRadio_IsPlaying();
void WebRadio_SetBitrate(uint32_t bitrate);
float WebRadio_GetDataRate();