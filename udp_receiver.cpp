#include "udp_receiver.h"
#include "config.h"
#include <ArduinoJson.h>

void UdpReceiver::begin(uint16_t listenPort) {
    _port = listenPort;
    if (_udp.begin(_port)) {
        Serial.printf("[UDP] Listening on port %u\n", _port);
    } else {
        Serial.println("[UDP] Failed to start UDP listener");
    }
}

void UdpReceiver::setHandler(PacketHandler handler) {
    _handler = handler;
}

void UdpReceiver::loop() {
    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) {
        return;
    }

    _processPacket();
}

void UdpReceiver::_processPacket() {
    // Read packet into buffer
    const size_t BUF_SIZE = 1024;
    static char buffer[BUF_SIZE];

    int len = _udp.read(buffer, BUF_SIZE - 1);
    if (len <= 0) {
        return;
    }
    buffer[len] = '\0';

    // Parse JSON
    StaticJsonDocument<2048> doc;
    DeserializationError err = deserializeJson(doc, buffer);

    if (err) {
        Serial.print("[UDP] JSON parse error: ");
        Serial.println(err.c_str());
        return;
    }

    SpectrumPacket pkt{};
    pkt.numBands = 0;

    // BPM (may be null)
    if (doc.containsKey("bpm") && !doc["bpm"].isNull()) {
        pkt.bpm = doc["bpm"].as<float>();
    } else {
        pkt.bpm = -1.0f;  // unknown
    }

    // Bands array
    if (doc.containsKey("bands") && doc["bands"].is<JsonArray>()) {
        JsonArray arr = doc["bands"].as<JsonArray>();
        size_t count = min((size_t)arr.size(), NUM_BINS);
        pkt.numBands = count;
        for (size_t i = 0; i < count; ++i) {
            pkt.bands[i] = arr[i].as<float>();
        }
    } else {
        Serial.println("[UDP] Packet missing 'bands' array");
        return;
    }

    if (_handler) {
        _handler(pkt);
    }
}