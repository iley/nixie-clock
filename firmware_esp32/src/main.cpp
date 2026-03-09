#include <Arduino.h>

#include "ota.h"
#include "rtc.h"
#include "sync.h"
#include "tubes.h"

// Stores the last sync day number to avoid repeatedly re-syncing at 04:00.
static int last_sync_yday;

// Whether NTP sync has ever succeeded (i.e. WiFi is reachable).
static bool ntp_available = false;

// --- Button debouncing ---

static const int kButtonA = 34;
static const int kButtonB = 35;
static const unsigned long kDebounceMs = 100;

struct Button {
  bool was_pressed;
  bool pressed;
  unsigned long last_change_millis;
};

static Button left_button;
static Button right_button;

void buttonInit(Button* btn) {
  btn->was_pressed = false;
  btn->pressed = false;
  btn->last_change_millis = 0;
}

void buttonUpdate(Button* btn, bool input) {
  btn->was_pressed = btn->pressed;
  if (millis() - btn->last_change_millis < kDebounceMs) {
    return;
  }
  btn->pressed = input;
  if (btn->was_pressed != btn->pressed) {
    btn->last_change_millis = millis();
  }
}

// --- Time setting state ---

static const unsigned long kBlinkMs = 250;

static bool running = true;
static int current_digit;
static unsigned long setting_start_millis;
static byte digits[4];  // HHMM being edited (local time)

// --- Forward declarations ---

void outputDateTime(struct tm *);
void syncClock();
void printDateTime(struct tm *datetime);
void enterTimeSetting(struct tm *local);
void handleTimeSetting();

void setup() {
  last_sync_yday = -1;

  Serial.begin(115200);

  Serial.println("Initializing GPIO");
  tubesSetup();
  otaSetup();

  buttonInit(&left_button);
  buttonInit(&right_button);

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

  // Update button state.
  buttonUpdate(&left_button, digitalRead(kButtonA) == LOW);
  buttonUpdate(&right_button, digitalRead(kButtonB) == LOW);

  bool left_up = !left_button.pressed && left_button.was_pressed;
  bool right_up = !right_button.pressed && right_button.was_pressed;

  if (running) {
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

    if (left_up) {
      enterTimeSetting(&local);
      return;
    }

    if (right_up && ntp_available) {
      Serial.println("Manual NTP sync requested");
      // Blink digits left to right as visual feedback.
      byte d[4] = {
        (byte)(local.tm_hour / 10), (byte)(local.tm_hour % 10),
        (byte)(local.tm_min / 10),  (byte)(local.tm_min % 10),
      };
      for (int i = 0; i < 4; i++) {
        byte saved = d[i];
        d[i] = 10;  // blank
        outputTubeDigits(d[0], d[1], d[2], d[3]);
        delay(150);
        d[i] = saved;
        outputTubeDigits(d[0], d[1], d[2], d[3]);
        delay(50);
      }
      syncClock();
      return;
    }

    outputDateTime(&local);
  } else {
    handleTimeSetting();
  }
}

void enterTimeSetting(struct tm *local) {
  running = false;
  current_digit = 0;
  setting_start_millis = millis();

  digits[0] = local->tm_hour / 10;
  digits[1] = local->tm_hour % 10;
  digits[2] = local->tm_min / 10;
  digits[3] = local->tm_min % 10;

  Serial.println("Entering time setting mode");
}

void handleTimeSetting() {
  bool left_up = !left_button.pressed && left_button.was_pressed;
  bool right_up = !right_button.pressed && right_button.was_pressed;

  if (left_up) {
    if (current_digit < 3) {
      current_digit++;
    } else {
      // Done setting time. Convert local digits back to UTC and store.
      struct tm local = {};
      local.tm_hour = digits[0] * 10 + digits[1];
      local.tm_min = digits[2] * 10 + digits[3];
      local.tm_sec = 0;

      // Preserve current date from RTC.
      struct tm now;
      rtcGetCurrentTime(&now);
      local.tm_year = now.tm_year;
      local.tm_mon = now.tm_mon;
      local.tm_mday = now.tm_mday;

      struct tm utc;
      localToUtc(&local, &utc);
      rtcSetCurrentTime(&utc);

      running = true;
      Serial.println("Time set");
      return;
    }
  } else if (right_up) {
    byte limit = 10;
    if (current_digit == 0) {
      limit = 3;
    } else if (current_digit == 1 && digits[0] == 2) {
      limit = 4;
    } else if (current_digit == 2) {
      limit = 6;
    }
    digits[current_digit] = (digits[current_digit] + 1) % limit;

    // Constrain hours digit 1 when digit 0 becomes 2.
    if (digits[0] == 2 && digits[1] > 3) {
      digits[1] = 3;
    }
  }

  // Display digits with current one blinking.
  byte display[4];
  for (int i = 0; i < 4; i++) {
    display[i] = digits[i];
  }
  if (((millis() - setting_start_millis) / kBlinkMs) % 2 == 1) {
    display[current_digit] = 10;  // 10 = blank
  }
  outputTubeDigits(display[0], display[1], display[2], display[3]);
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

  ntp_available = true;
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
