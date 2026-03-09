#include "ota.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "secret_wifi.h"
#include "tubes.h"

static const int kButtonA = 34;
static const int kButtonB = 35;
static const unsigned long kHoldDurationMs = 3000;

static unsigned long both_pressed_since = 0;

void otaSetup() {
  pinMode(kButtonA, INPUT);
  pinMode(kButtonB, INPUT);
}

bool otaCheckButtons() {
  if (digitalRead(kButtonA) == LOW && digitalRead(kButtonB) == LOW) {
    if (both_pressed_since == 0) {
      both_pressed_since = millis();
    } else if (millis() - both_pressed_since >= kHoldDurationMs) {
      return true;
    }
  } else {
    both_pressed_since = 0;
  }
  return false;
}

void otaEnterMode() {
  Serial.println("Entering OTA mode");
  outputTubeDigits(9, 9, 9, 9);

  WiFi.begin(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
  for (int i = 0; i < 60; i++) {
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi failed, restarting");
    ESP.restart();
  }

  ArduinoOTA.setHostname("nixie-clock");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA update starting");
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("OTA update complete");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", progress * 100 / total);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error %u: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("receive failed");
    else if (error == OTA_END_ERROR) Serial.println("end failed");
  });

  ArduinoOTA.begin();
  Serial.print("OTA ready. IP: ");
  Serial.println(WiFi.localIP());

  while (true) {
    ArduinoOTA.handle();
    outputTubeDigits(9, 9, 9, 9);
    delay(10);
  }
}
