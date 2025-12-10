#pragma once

#include <WiFi.h>
#include "config.h"

class WifiManager {
public:
    void begin();
    void loop();

    bool isConnected() const {
        return WiFi.status() == WL_CONNECTED;
    }

private:
    uint32_t _lastAttemptMs = 0;

    void _connectOnce();
};