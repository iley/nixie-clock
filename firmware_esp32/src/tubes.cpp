#include "tubes.h"

constexpr const int tubeCount = 4;
constexpr const int pinsPerTube = 4;

byte tubePins[tubeCount * pinsPerTube] = {
// A   B   C   D
  15,  2,  4, 16, // 1
  17,  5, 18, 19, // 2
  12, 14, 27, 26, // 3
  25, 33, 32, 23, // 4
};

const byte kDigitsMap[] = { 6, 4, 5, 1, 0, 9, 8, 2, 3, 7, 0xf };

void tubesSetup() {
  for (int i = 0; i < tubeCount * pinsPerTube; i++) {
    pinMode(tubePins[i], OUTPUT);
  }
}

void outputTubeDigit(int tubeIndex, byte value) {
  byte mappedValue = kDigitsMap[value];
  byte* pins = tubePins + (tubeIndex * pinsPerTube);
  digitalWrite(pins[0], mappedValue & 1);
  digitalWrite(pins[1], (mappedValue >> 1) & 1);
  digitalWrite(pins[2], (mappedValue >> 2) & 1);
  digitalWrite(pins[3], (mappedValue >> 3) & 1);
}

void outputTubeDigits(byte d1, byte d2, byte d3, byte d4) {
  outputTubeDigit(2, d1);
  outputTubeDigit(3, d2);
  outputTubeDigit(0, d3);
  outputTubeDigit(1, d4);
}
