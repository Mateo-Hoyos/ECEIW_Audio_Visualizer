#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "udp_receiver.h"
#include "teensy_link.h"
#include "types.h"

// Serial2 is the hardware UART we use to talk to the Teensy
TeensyLink   teensyLink(Serial2);
WifiManager  wifiManager;
UdpReceiver  udpReceiver;

// Forward declaration of our packet handler
void onSpectrumPacket(const SpectrumPacket& pkt);

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n=== ESP32 Audio Visualizer Bridge ===");

    wifiManager.begin();

    teensyLink.begin();

    udpReceiver.begin(UDP_LISTEN_PORT);
    udpReceiver.setHandler(onSpectrumPacket);
}

void loop() {
    wifiManager.loop();
    udpReceiver.loop();

    // Maybe a heartbeat LED or debug prints here
}

// Called whenever a valid SpectrumPacket arrives from the Pi
void onSpectrumPacket(const SpectrumPacket& pkt) {
    // Debug print (optional)
    Serial.print("[Packet] BPM: ");
    Serial.print(pkt.bpm);
    Serial.print("  Bands[0]: ");
    Serial.println(pkt.bands[0], 3);

    // Forward to Teensy in compact binary format
    teensyLink.sendSpectrum(pkt);
}