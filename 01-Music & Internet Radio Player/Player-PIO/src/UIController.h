#pragma once
#include "Arduino.h"
#include "ui.h"
#include "AudioPlayer.h"

// Initialize the UI controller
void UIController_Init();

// Process UI events in a loop
void UIController_Loop();

// Update UI elements with current state
void UIController_UpdateUI();

// Set up event handlers for UI elements
void UIController_SetupEvents();

// Update the time display
void UIController_UpdateTimeDisplay();

// Update the progress bar
void UIController_UpdateProgressBar();

// Update the file/station list
void UIController_UpdateList();

// Callback functions for UI events
void UI_ModeButtonCallback(lv_event_t *e);
void UI_PlayPauseButtonCallback(lv_event_t *e);
void UI_NextButtonCallback(lv_event_t *e);
void UI_PreviousButtonCallback(lv_event_t *e);
void UI_ListCallback(lv_event_t *e);

// New callbacks for volume and brightness controls
void UI_VolumeArcCallback(lv_event_t *e);
void UI_BrightnessArcCallback(lv_event_t *e);