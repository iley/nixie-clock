#ifndef _NIXIE_CLOCK_RTC_H
#define _NIXIE_CLOCK_RTC_H

#include <Arduino.h>
#include <bb_rtc.h>

bool rtcSetup();
void rtcGetCurrentTime(struct tm* datetime);
void rtcSetCurrentTime(struct tm* datetime);

#endif // _NIXIE_CLOCK_RTC_H
