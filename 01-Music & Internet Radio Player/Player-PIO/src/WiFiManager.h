#pragma once
#include "Arduino.h"
#include <WiFi.h>
#include "config.h"

// Function declarations
bool WiFi_Init();
bool WiFi_IsConnected();
void WiFi_Update();      // Non-blocking status update
void WiFi_Reconnect();
void WiFi_PrintStatus();
int WiFi_GetRSSI(); 