#include "WiFiManager.h"
#include "esp_task_wdt.h"

bool wifi_available = false;
bool wifi_init_started = false;
unsigned long last_reconnect_attempt = 0;
const unsigned long reconnect_interval = 30000; // Try to reconnect every 30 seconds

// Non-blocking WiFi initialization - starts connection and returns immediately
bool WiFi_Init() {
    Serial.println("Starting WiFi (non-blocking)...");

    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);

    // Begin WiFi connection - this returns immediately
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    wifi_init_started = true;

    // Quick check - try for max 2 seconds during boot (non-critical)
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_time < 2000)) {
        delay(100);
        esp_task_wdt_reset(); // Feed watchdog
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        wifi_available = true;
        return true;
    }

    // Not connected yet - will continue in background
    Serial.println("WiFi connecting in background...");
    wifi_available = false;
    return false; // Not an error - connection continues in background
}

// Check and update WiFi status (call periodically from task)
void WiFi_Update() {
    if (!wifi_init_started) return;

    if (WiFi.status() == WL_CONNECTED && !wifi_available) {
        Serial.println("WiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        wifi_available = true;
    } else if (WiFi.status() != WL_CONNECTED && wifi_available) {
        Serial.println("WiFi disconnected");
        wifi_available = false;
    }
}

// Check if WiFi is currently connected
bool WiFi_IsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// Non-blocking reconnect - just triggers reconnection
void WiFi_Reconnect() {
    // Only attempt reconnection at intervals
    if (millis() - last_reconnect_attempt < reconnect_interval) {
        return;
    }

    last_reconnect_attempt = millis();

    // Check if already connected
    if (WiFi.status() == WL_CONNECTED) {
        wifi_available = true;
        return;
    }

    Serial.println("Triggering WiFi reconnect...");

    // Disconnect and reconnect - non-blocking
    WiFi.disconnect(false); // Don't erase credentials
    WiFi.reconnect(); // This returns immediately

    wifi_available = false;
}

// Print the current WiFi status
void WiFi_PrintStatus() {
    switch (WiFi.status()) {
        case WL_CONNECTED:
            Serial.println("WiFi: Connected");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
            Serial.print("RSSI: ");
            Serial.println(WiFi.RSSI());
            break;
        case WL_NO_SHIELD:
            Serial.println("WiFi: No shield");
            break;
        case WL_IDLE_STATUS:
            Serial.println("WiFi: Idle");
            break;
        case WL_NO_SSID_AVAIL:
            Serial.println("WiFi: No SSID available");
            break;
        case WL_SCAN_COMPLETED:
            Serial.println("WiFi: Scan completed");
            break;
        case WL_CONNECT_FAILED:
            Serial.println("WiFi: Connection failed");
            break;
        case WL_CONNECTION_LOST:
            Serial.println("WiFi: Connection lost");
            break;
        case WL_DISCONNECTED:
            Serial.println("WiFi: Disconnected");
            break;
        default:
            Serial.println("WiFi: Unknown status");
            break;
    }
}

// Get the WiFi signal strength (RSSI)
int WiFi_GetRSSI() {
    if (WiFi.status() == WL_CONNECTED) {
        return WiFi.RSSI();
    } else {
        return -100; // Very low value to indicate no connection
    }
}