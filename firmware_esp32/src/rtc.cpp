#include "rtc.h"

static BBRTC rtc;

constexpr byte PIN_SDA = 21;
constexpr byte PIN_SCL = 22;

bool rtcSetup() {
  int err = rtc.init(PIN_SDA, PIN_SCL);
  if (err != RTC_SUCCESS) {
    return false;
  }
  return true;
}

void rtcGetCurrentTime(struct tm* datetime) {
  rtc.getTime(datetime);
}

void rtcSetCurrentTime(struct tm* datetime) {
  rtc.setTime(datetime);
}
