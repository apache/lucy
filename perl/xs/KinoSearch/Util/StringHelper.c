#include "xs/XSBind.h"
#include "KinoSearch/Util/StringHelper.h"

// TODO: replace with code from ICU in common/ucnv_u8.c.
chy_bool_t
kino_StrHelp_utf8_valid(const char *ptr, size_t size)
{
    const U8 *uptr = (const U8*)ptr;
    return is_utf8_string(uptr, size);
}


