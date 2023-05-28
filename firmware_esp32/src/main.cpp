#include <Arduino.h>

constexpr const int tubeCount = 4;
constexpr const int pinsPerTube = 4;

byte tubePins[tubeCount * pinsPerTube] = {
// A   B   C   D
  15,  2,  4, 16, // 1
  17,  5, 18, 19, // 2
  27, 26, 25, 23,  // 3
  35, 34, 39, 36,  // 4
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

  /*
  for (int i = 0; i < tubeCount * pinsPerTube; i++) {
    digitalWrite(tubePins[i], 1);
  }
  Serial.println("ON");
  delay(1000);

  for (int i = 0; i < tubeCount * pinsPerTube; i++) {
    digitalWrite(tubePins[i], 0);
  }
  Serial.println("OFF");
  delay(1000);
  */
}
