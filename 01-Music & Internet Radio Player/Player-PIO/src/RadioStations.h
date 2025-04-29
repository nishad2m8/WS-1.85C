#pragma once
#include "Arduino.h"

// Radio Station Structure
typedef struct {
    const char* name;
    const char* url;
} RadioStation;

// Radio Stations List - SIMPLIFIED WITH STABLE STREAM URLS
const RadioStation radioStations[] = {
    {"BBC World Service", "http://stream.live.vc.bbcmedia.co.uk/bbc_world_service"},
    {"Smooth Jazz", "http://ice1.somafm.com/smoothjazz-128-mp3"},
    {"Classic FM", "http://media-ice.musicradio.com/ClassicFMMP3"},
    {"Chill Radio", "http://ice1.somafm.com/defcon-128-mp3"},
    {"80s Hits", "http://ice1.somafm.com/u80s-128-mp3"},
    {"Jazz Radio", "http://ice1.somafm.com/gsclassic-128-mp3"},
    {"Classical", "http://stream.srg-ssr.ch/m/rsc_de/mp3_128"},
    {"Lounge", "http://ice1.somafm.com/lush-128-mp3"},
    {"Rock", "http://stream.rockantenne.de/rockantenne/stream/mp3"}
};

// Number of radio stations
const int RADIO_STATION_COUNT = sizeof(radioStations) / sizeof(RadioStation);