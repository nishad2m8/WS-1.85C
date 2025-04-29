#pragma once

// Audio Configuration For ESP32
// This file should be included before Audio.h in any file that uses audio

// Increase performance by using PSRAM if available
#if defined(ESP32) && defined(CONFIG_SPIRAM_SUPPORT)
  #define AUDIO_USE_PSRAM true
#else
  #define AUDIO_USE_PSRAM false
#endif

// Note: This particular ESP32 Audio library uses direct property access
// instead of setter methods, so the following are just hints for consistent configuration.
// You need to set audio.inBufferSize = 2048 directly in your code.

// Optimized Audio settings for memory-constrained environments
#define DEFAULT_AUDIO_BUFFER_SIZE 2048  // Use a much smaller buffer than default
#define AUDIO_RUNNING_CORE 0            // Use Core 0 for audio processing
#define AUDIO_TASK_PRIORITY 4           // High priority for audio task
#define AUDIO_TASK_STACK_SIZE 3072      // Reduced stack size