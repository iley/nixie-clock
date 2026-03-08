#ifndef _NIXIE_CLOCK_SYNC_H
#define _NIXIE_CLOCK_SYNC_H

#include "rtc.h"

bool fetchCurrentTimeFromNtp(struct tm* datetime);
void utcToLocal(const struct tm* utc, struct tm* local);

#endif // _NIXIE_CLOCK_SYNC_H
