#define C_KINO_ATOMIC
#define KINO_USE_SHORT_NAMES
#include "KinoSearch/Util/Atomic.h"

/********************************** Windows ********************************/
#ifdef CHY_HAS_WINDOWS_H
#include <windows.h>

chy_bool_t
kino_Atomic_wrapped_cas_ptr(void *volatile *target, void *old_value, 
                            void *new_value)
{
    return InterlockedCompareExchangePointer(target, new_value, old_value) 
        == old_value;
}

/************************** Fall back to ptheads ***************************/
#elif defined(CHY_HAS_PTHREAD_H)

#include <pthread.h>
pthread_mutex_t kino_Atomic_mutex = PTHREAD_MUTEX_INITIALIZER;

#endif

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

