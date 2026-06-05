#pragma once
#include "Arduino.h"

// Radio Station Structure
typedef struct {
    const char* name;
    const char* url;
} RadioStation;

// Radio Stations List 
const RadioStation radioStations[] = {
    {"Test-1", "http://--"},
    {"Testt-2", "https://--"}
};

// Number of radio stations
const int RADIO_STATION_COUNT = sizeof(radioStations) / sizeof(RadioStation);