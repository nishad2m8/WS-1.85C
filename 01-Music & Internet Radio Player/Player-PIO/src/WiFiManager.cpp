#include "WiFiManager.h"

bool wifi_available = false;
unsigned long last_reconnect_attempt = 0;
const unsigned long reconnect_interval = 30000; // Try to reconnect every 30 seconds

// Initialize WiFi connection
bool WiFi_Init() {
    Serial.println("Initializing WiFi...");
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Begin WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait for connection with timeout
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        // Check for timeout
        if (millis() - start_time > WIFI_TIMEOUT) {
            Serial.println("\nWiFi connection timed out!");
            wifi_available = false;
            return false;
        }
    }
    
    // Successfully connected
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    wifi_available = true;
    
    return true;
}

// Check if WiFi is currently connected
bool WiFi_IsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

// Attempt to reconnect WiFi if disconnected
void WiFi_Reconnect() {
    // Only attempt reconnection at intervals to avoid blocking
    if (millis() - last_reconnect_attempt < reconnect_interval) {
        return;
    }
    
    last_reconnect_attempt = millis();
    
    // Check if already connected
    if (WiFi.status() == WL_CONNECTED) {
        wifi_available = true;
        return;
    }
    
    Serial.println("Attempting to reconnect WiFi...");
    
    // Disconnect first
    WiFi.disconnect();
    delay(1000);
    
    // Try to reconnect
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    // Wait briefly for connection
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        
        // Short timeout for non-blocking behavior
        if (millis() - start_time > 5000) {
            Serial.println("\nReconnection attempt timed out");
            wifi_available = false;
            return;
        }
    }
    
    Serial.println("\nWiFi reconnected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    wifi_available = true;
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