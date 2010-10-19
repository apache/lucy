#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Analysis/Stopalizer.h"
#include "KinoSearch/Object/Host.h"

Hash*
Stopalizer_gen_stoplist(const CharBuf *language)
{
    return (Hash*)Host_callback_obj(STOPALIZER, "gen_stoplist", 1,
        ARG_STR("language", language));
}


