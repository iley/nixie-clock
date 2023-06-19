#ifndef _NIXIE_CLOCK_SYNC_H
#define _NIXIE_CLOCK_SYNC_H

#include "rtc.h"

bool fetchCurrentTimeFromNtp(struct tm* datetime);

#endif // _NIXIE_CLOCK_SYNC_H
