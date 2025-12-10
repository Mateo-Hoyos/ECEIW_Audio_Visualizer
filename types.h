#pragma once

#include <Arduino.h>
#include "config.h"

// Data coming from the Pi (after parsing JSON)
struct SpectrumPacket {
    float   bpm;                  // BPM estimate from Pi (or -1 if unknown)
    float   bands[NUM_BINS];      // 32 normalized floats [0.0, 1.0]
    size_t  numBands;             // actual number of bands filled (<= NUM_BINS)
};

// Data format sent to the Teensy (binary, on Serial2)
// Layout:
//   [0]  = FRAME_START_1 (0xAA)
//   [1]  = FRAME_START_2 (0x55)
//   [2]  = frameId low byte
//   [3]  = frameId high byte
//   [4]  = bpmTimes10 low byte  (uint16_t, bpm * 10, 0 if unknown)
//   [5]  = bpmTimes10 high byte
//   [6..(6+NUM_BINS-1)] = band magnitudes as uint8 (0-255)
//   [6+NUM_BINS]        = checksum (sum of all bytes except start bytes)
struct TeensyFrame {
    uint16_t frameId;
    uint16_t bpmTimes10;
    uint8_t  bands[NUM_BINS];
    uint8_t  checksum;
};