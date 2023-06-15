#include <Arduino.h>

constexpr const int tubeCount = 4;
constexpr const int pinsPerTube = 4;

byte tubePins[tubeCount * pinsPerTube] = {
// A   B   C   D
  15,  2,  4, 16, // 1
  17,  5, 18, 19, // 2
  12, 14, 27, 26, // 3
  25, 33, 32, 23, // 4
};

void outputTubeDigit(int tubeIndex, byte value) {
  byte* pins = tubePins + (tubeIndex * pinsPerTube);
  digitalWrite(pins[0], value & 1);
  digitalWrite(pins[1], (value >> 1) & 1);
  digitalWrite(pins[2], (value >> 2) & 1);
  digitalWrite(pins[3], (value >> 3) & 1);
}

void setup() {
  for (int i = 0; i < tubeCount * pinsPerTube; i++) {
    pinMode(tubePins[i], OUTPUT);
  }
}

void loop() {
  static int digit = 0;

  for (int tubeIndex = 0; tubeIndex < tubeCount; tubeIndex++) {
    outputTubeDigit(tubeIndex, digit);
  }

  digit = (digit + 1) % 10;

  delay(500);
}
