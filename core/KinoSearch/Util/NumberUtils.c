#define C_KINO_NUMBERUTILS
#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include <string.h>

#include "KinoSearch/Util/NumberUtils.h"

const uint8_t NumUtil_u1masks[8] = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

const uint8_t NumUtil_u2shifts[4] = { 0x0, 0x2, 0x4,  0x6  };
const uint8_t NumUtil_u2masks[4]  = { 0x3, 0xC, 0x30, 0xC0 };

const uint8_t NumUtil_u4shifts[2] = { 0x00, 0x04 };
const uint8_t NumUtil_u4masks[2]  = { 0x0F, 0xF0 };


