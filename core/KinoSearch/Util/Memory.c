#define C_KINO_MEMORY
#include <stdlib.h>
#include <stdio.h>
#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES
#include "KinoSearch/Util/Memory.h"

void*
Memory_wrapped_malloc(size_t count)
{
    void *pointer = malloc(count);
    if (pointer == NULL && count != 0) {
        fprintf(stderr, "Can't malloc %" U64P " bytes.\n", (uint64_t)count);
        exit(1);
    }
    return pointer;
}

void*
Memory_wrapped_calloc(size_t count, size_t size)
{
    void *pointer = calloc(count, size);
    if (pointer == NULL && count != 0) {
        fprintf(stderr, "Can't calloc %" U64P " elements of size %" U64P ".\n", 
            (uint64_t)count, (uint64_t)size);
        exit(1);
    }
    return pointer;
}

void*
Memory_wrapped_realloc(void *ptr, size_t size)
{
    void *pointer = realloc(ptr, size);
    if (pointer == NULL && size != 0) {
        fprintf(stderr, "Can't realloc %" U64P " bytes.\n", (uint64_t)size);
        exit(1);
    }
    return pointer;
}

void
kino_Memory_wrapped_free(void *ptr)
{
    free(ptr);
}

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

size_t
kino_Memory_oversize(size_t minimum, size_t width)
{
    // For larger arrays, grow by an excess of 1/8; grow faster when the array
    // is small.
    size_t extra = minimum / 8;
    if (extra < 3) {
        extra = 3;
    }
    size_t amount = minimum + extra;

    // Detect wraparound and return SIZE_MAX instead. 
    if (amount + 7 < minimum) { 
        return SIZE_MAX;
    }

    // Round up for small widths so that the number of bytes requested will be
    // a multiple of the machine's word size.
    if (sizeof(size_t) == 8) { // 64-bit
        switch(width) {
        case 1:  amount = (amount + 7) & CHY_I64_C(0xFFFFFFFFFFFFFFF8); break;
        case 2:  amount = (amount + 3) & CHY_I64_C(0xFFFFFFFFFFFFFFFC); break;
        case 4:  amount = (amount + 1) & CHY_I64_C(0xFFFFFFFFFFFFFFFE); break;
        default: break;
        }
    }
    else { // 32-bit
        switch(width) {
        case 1:  amount = (amount + 3) & ((size_t)0xFFFFFFFC); break;
        case 2:  amount = (amount + 1) & ((size_t)0xFFFFFFFE); break;
        default: break;
        }
    }

    return amount;
}


