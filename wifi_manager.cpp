#include "wifi_manager.h"

void WifiManager::begin() {
    Serial.println("[WiFi] Initializing...");

    WiFi.mode(WIFI_STA);

    // Set static IP
    if (!WiFi.config(ESP_LOCAL_IP, ESP_GATEWAY, ESP_SUBNET)) {
        Serial.println("[WiFi] Failed to configure static IP");
    }

    _connectOnce();
}

void WifiManager::_connectOnce() {
    Serial.printf("[WiFi] Connecting to SSID: %s\n", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void WifiManager::loop() {
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }

    uint32_t now = millis();
    if (now - _lastAttemptMs >= WIFI_RECONNECT_INTERVAL_MS) {
        _lastAttemptMs = now;
        Serial.println("[WiFi] Not connected, retrying...");
        _connectOnce();
    }
}