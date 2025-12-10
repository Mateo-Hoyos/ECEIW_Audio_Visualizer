#pragma once

#include <HardwareSerial.h>
#include "types.h"

class TeensyLink {
public:
    TeensyLink(HardwareSerial& serialPort);

    void begin();
    void sendSpectrum(const SpectrumPacket& pkt);

private:
    HardwareSerial& _serial;
    uint16_t        _frameId = 0;

    uint8_t _computeChecksum(const TeensyFrame& frame);
};