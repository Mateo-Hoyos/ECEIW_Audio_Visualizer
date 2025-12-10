#include "teensy_link.h"
#include "config.h"

TeensyLink::TeensyLink(HardwareSerial& serialPort)
    : _serial(serialPort) {}

void TeensyLink::begin() {
    _serial.begin(TEENSY_BAUD, SERIAL_8N1, TEENSY_RX_PIN, TEENSY_TX_PIN);
    Serial.println("[Teensy] UART initialized");
}

uint8_t TeensyLink::_computeChecksum(const TeensyFrame& frame) {
    // Sum bpmTimes10 + bands + frameId, modulo 256
    uint32_t sum = 0;

    sum += (frame.frameId & 0xFF);
    sum += ((frame.frameId >> 8) & 0xFF);
    sum += (frame.bpmTimes10 & 0xFF);
    sum += ((frame.bpmTimes10 >> 8) & 0xFF);

    for (size_t i = 0; i < NUM_BINS; ++i) {
        sum += frame.bands[i];
    }

    return static_cast<uint8_t>(sum & 0xFF);
}

void TeensyLink::sendSpectrum(const SpectrumPacket& pkt) {
    TeensyFrame frame{};
    frame.frameId = _frameId++;

    // Encode BPM as tenths (e.g., 128.3 -> 1283). 0 means "unknown".
    if (pkt.bpm > 0.0f) {
        float bpmTimes10f = pkt.bpm * 10.0f;
        if (bpmTimes10f > 65535.0f) {
            bpmTimes10f = 65535.0f;
        }
        frame.bpmTimes10 = static_cast<uint16_t>(bpmTimes10f + 0.5f);
    } else {
        frame.bpmTimes10 = 0;
    }

    // Map normalized float bands [0,1] -> uint8 [0,255]
    for (size_t i = 0; i < NUM_BINS; ++i) {
        float v = (i < pkt.numBands) ? pkt.bands[i] : 0.0f;
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        uint8_t b = static_cast<uint8_t>(v * 255.0f + 0.5f);
        frame.bands[i] = b;
    }

    frame.checksum = _computeChecksum(frame);

    // Write to UART in the documented order
    _serial.write(FRAME_START_1);
    _serial.write(FRAME_START_2);

    _serial.write(static_cast<uint8_t>(frame.frameId & 0xFF));
    _serial.write(static_cast<uint8_t>((frame.frameId >> 8) & 0xFF));

    _serial.write(static_cast<uint8_t>(frame.bpmTimes10 & 0xFF));
    _serial.write(static_cast<uint8_t>((frame.bpmTimes10 >> 8) & 0xFF));

    _serial.write(frame.bands, NUM_BINS);

    _serial.write(frame.checksum);

    _serial.flush();  // ensure it actually goes out
}