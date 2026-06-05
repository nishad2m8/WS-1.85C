#pragma once

// WiFi
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define WIFI_TIMEOUT 10000

// Timezone
#define TIMEZONE_POSIX "TM-Zone"
#define NTP_SERVER1 "pool.ntp.org"
#define NTP_SERVER2 "time.nist.gov"

// Audio
#define DEFAULT_VOLUME 10
#define MAX_VOLUME 21

// Display
#define DEFAULT_BRIGHTNESS 40
#define MIN_BRIGHTNESS 10
#define MAX_BRIGHTNESS 100

// V2 Hardware
#ifdef HARDWARE_V2
    #define ES8311_I2C_ADDR     0x18
    #define ES8311_SAMPLE_RATE  44100
    #define ES8311_MCLK_MULTI   256
    #define ES7210_I2C_ADDR     0x40

    #define I2S_DOUT_PIN        47
    #define I2S_DIN_PIN         39
    #define I2S_BCLK_PIN        48
    #define I2S_LRC_PIN         38
    #define I2S_MCLK_PIN        2
    #define AMP_ENABLE_PIN      15
#endif
