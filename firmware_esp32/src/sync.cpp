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
  time_t local_time = localTimezone.toLocal(epoch_time);

  gmtime_r((time_t*)&local_time, datetime);

  timeClient.end();
  WiFi.disconnect(true);

  return true;
}
