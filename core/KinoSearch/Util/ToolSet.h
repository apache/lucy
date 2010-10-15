#ifndef H_KINO_TOOLSET
#define H_KINO_TOOLSET 1

#ifdef __cplusplus
extern "C" {
#endif

/** ToolSet groups together several commonly used header files, so that only
 * one pound-include directive is needed for them.
 *
 * It should only be used internally, and only included in C files rather than
 * header files, so that the header files remain as sparse as possible.
 */

#define KINO_USE_SHORT_NAMES
#define CHY_USE_SHORT_NAMES

#include "charmony.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "KinoSearch/Object/Obj.h"
#include "KinoSearch/Object/BitVector.h"
#include "KinoSearch/Object/ByteBuf.h"
#include "KinoSearch/Object/CharBuf.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Object/Hash.h"
#include "KinoSearch/Object/I32Array.h"
#include "KinoSearch/Object/Num.h"
#include "KinoSearch/Object/VArray.h"
#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Util/NumberUtils.h"
#include "KinoSearch/Util/Memory.h"
#include "KinoSearch/Util/StringHelper.h"

#ifdef __cplusplus
}
#endif

#endif // H_KINO_TOOLSET 


