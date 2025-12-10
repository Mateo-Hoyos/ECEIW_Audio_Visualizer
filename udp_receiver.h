#pragma once

#include <WiFiUdp.h>
#include "types.h"

class UdpReceiver {
public:
    using PacketHandler = void (*)(const SpectrumPacket& pkt);

    void begin(uint16_t listenPort);
    void setHandler(PacketHandler handler);
    void loop();

private:
    WiFiUDP      _udp;
    uint16_t     _port = 0;
    PacketHandler _handler = nullptr;

    void _processPacket();
};