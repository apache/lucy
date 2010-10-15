#define C_KINO_SLEEP
#include "KinoSearch/Util/Sleep.h"

/********************************* WINDOWS ********************************/
#ifdef CHY_HAS_WINDOWS_H

#include <windows.h>

void
kino_Sleep_sleep(uint32_t seconds)
{
    Sleep(seconds * 1000);
}

void
kino_Sleep_millisleep(uint32_t milliseconds)
{
    Sleep(milliseconds);
}

/********************************* UNIXEN *********************************/
#elif defined(CHY_HAS_UNISTD_H)

#include <unistd.h>

void
kino_Sleep_sleep(uint32_t seconds)
{
    sleep(seconds);
}

void
kino_Sleep_millisleep(uint32_t milliseconds)
{
    uint32_t seconds = milliseconds / 1000;
    milliseconds  = milliseconds % 1000;
    sleep(seconds);
    // TODO: probe for usleep. 
    usleep(milliseconds * 1000);
}

#else
  #error "Can't find a known sleep API."
#endif // OS switch. 


