#pragma once

#include <Arduino.h>
#include <IPAddress.h>

// -------- WiFi configuration --------
// Pi hotspot SSID and password
static const char* WIFI_SSID     = "AuViPi";
static const char* WIFI_PASSWORD = "auvi";

// Give ESP32 a fixed IP in the Pi's network
static const IPAddress ESP_LOCAL_IP(192, 168, 4, 2);
static const IPAddress ESP_GATEWAY(192, 168, 4, 1);
static const IPAddress ESP_SUBNET (255, 255, 255, 0);

// How often to attempt reconnect if WiFi drops (ms)
static const uint32_t WIFI_RECONNECT_INTERVAL_MS = 5000;

// -------- UDP configuration --------
static const uint16_t UDP_LISTEN_PORT = 5005;

// -------- Teensy UART configuration --------
// UART to Teensy (we use Serial2 on the ESP32)
static const uint32_t TEENSY_BAUD = 1000000;   // 1 Mbps

// Pins for UART2 (change to match your wiring)
// You mentioned ESP32 Pin 17 (Tx) and Pin 18 (Rx).
static const int TEENSY_TX_PIN = 17;  // ESP32 TX -> Teensy RX (pin 0)
static const int TEENSY_RX_PIN = 18;  // ESP32 RX <- Teensy TX (pin 1)

// -------- Protocol configuration --------
static const size_t NUM_BINS = 32;

// Frame header bytes to mark start of packet to Teensy
static const uint8_t FRAME_START_1 = 0xAA;
static const uint8_t FRAME_START_2 = 0x55;