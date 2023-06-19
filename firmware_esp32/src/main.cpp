#include <Arduino.h>

#include "rtc.h"
#include "sync.h"
#include "tubes.h"

void outputDateTime(struct tm* now) {
  byte hour_high = now->tm_hour / 10;
  byte hour_low = now->tm_hour % 10;
  byte min_high = now->tm_min / 10;
  byte min_low = now->tm_min % 10;
  outputTubeDigits(hour_high, hour_low, min_high, min_low);
}

void setup() {
  Serial.begin(115200);

  Serial.println("Initializing GPIO");
  tubesSetup();

  Serial.print("Initializing RTC... ");
  bool success = rtcSetup();
  if (success) {
    Serial.println("DONE");
  } else {
    Serial.println("FAIL");
  }
}

void printDateTime(struct tm* datetime) {
  Serial.print(datetime.tm_hour);
  Serial.print(":");
  Serial.print(datetime.tm_min);
  Serial.print(":");
  Serial.print(datetime.tm_sec);
}

void syncClock() {
  struct tm now;

  bool success = fetchCurrentTimeFromNtp();
  if (!success) {
    Serial.println("Failed to fetch current time from NTP");
    return;
  }

  rtcSetCurrentTime(&now);
  Serial.print("Synced clock successfully. Current time: ");
  printDateTime(&now);
  Serial.println();
}

void loop() {
  struct tm now;
  rtcGetCurrentTime(&now);

  outputDateTime(&now);
  delay(100);
}
