#ifndef _NIXIE_CLOCK_RTC_H
#define _NIXIE_CLOCK_RTC_H

#include <stdint.h>

typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t weekDay;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} datetime_t;

void rtc_init();
void rtc_set(datetime_t *dt);
void rtc_get(datetime_t* dt);

#endif // _NIXIE_CLOCK_RTC_H
