#include "sync.h"

#include <Arduino.h>
#include <NTPClient.h>
#include <Timezone.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "secret_wifi.h"

static WiFiUDP ntpUdp;
static NTPClient timeClient(ntpUdp);

TimeChangeRule BST = {"BST", Last, Sun, Mar, 1, 60}; // Daylight Saving time = UTC + 1 hour
TimeChangeRule GMT = {"GMT", Last, Sun, Oct, 2, 0};  // Standard time = UTC
Timezone localTimezone(BST, GMT);

void utcToLocal(const struct tm* utc, struct tm* local) {
  // Convert struct tm (UTC) -> epoch -> local epoch -> struct tm.
  struct tm tmp = *utc;
  time_t utc_epoch = mktime(&tmp);
  time_t local_epoch = localTimezone.toLocal(utc_epoch);
  gmtime_r(&local_epoch, local);
}

bool fetchCurrentTimeFromNtp(struct tm* datetime) {
  Serial.println("Connecting to WiFi " SECRET_WIFI_SSID);
  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);

  for (int i = 0; i < 60; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    Serial.print( "." );
  }
  Serial.println();

  if ( WiFi.status() != WL_CONNECTED ) {
    Serial.println("Timeout");
    return false;
  }

  timeClient.begin();
  timeClient.update();

  time_t epoch_time = timeClient.getEpochTime();

  // Store UTC in the RTC; local conversion happens at display time.
  gmtime_r(&epoch_time, datetime);

  timeClient.end();
  WiFi.disconnect(true);

  return true;
}
