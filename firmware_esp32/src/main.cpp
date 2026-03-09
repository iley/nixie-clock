#include <Arduino.h>

#include "ota.h"
#include "rtc.h"
#include "sync.h"
#include "tubes.h"

// Stores the last sync day number to avoid repeatedly re-syncing at 00:00.
static int last_sync_yday;

void outputDateTime(struct tm *);
void syncClock();
void printDateTime(struct tm *datetime);

void setup() {
  last_sync_yday = -1;

  Serial.begin(115200);

  Serial.println("Initializing GPIO");
  tubesSetup();
  otaSetup();

  // Output zeroes to show that the clock is initializing.
  outputTubeDigits(0, 0, 0, 0);

  Serial.print("Initializing RTC... ");
  bool success = rtcSetup();
  if (success) {
    Serial.println("DONE");
  } else {
    Serial.println("FAIL");
  }

  Serial.print("Syncing clock...");
  syncClock();
}

void loop() {
  if (otaCheckButtons()) {
    otaEnterMode();
  }

  struct tm now;
  rtcGetCurrentTime(&now);

  // Sync the clock at 04:00 UTC every day.
  if (now.tm_hour == 4 && now.tm_min == 0 && now.tm_sec == 0 &&
      last_sync_yday != now.tm_yday) {
    syncClock();
    rtcGetCurrentTime(&now);
    last_sync_yday = now.tm_yday;
  }

  // RTC stores UTC; convert to local time (with DST) for display.
  struct tm local;
  utcToLocal(&now, &local);
  outputDateTime(&local);
}

void outputDateTime(struct tm *now) {
  byte hour_high = now->tm_hour / 10;
  byte hour_low = now->tm_hour % 10;
  byte min_high = now->tm_min / 10;
  byte min_low = now->tm_min % 10;
  outputTubeDigits(hour_high, hour_low, min_high, min_low);
}

void syncClock() {
  struct tm now;

  bool success = fetchCurrentTimeFromNtp(&now);
  if (!success) {
    Serial.println("Failed to fetch current time from NTP");
    return;
  }

  rtcSetCurrentTime(&now);

  struct tm local;
  utcToLocal(&now, &local);
  Serial.print("Synced clock successfully. Current time: ");
  printDateTime(&local);
  Serial.println();
}

void printDateTime(struct tm *datetime) {
  Serial.print(datetime->tm_hour);
  Serial.print(":");
  Serial.print(datetime->tm_min);
  Serial.print(":");
  Serial.print(datetime->tm_sec);
}
