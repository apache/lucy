#define C_KINO_POSTINGLIST
#include <string.h>

#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/PostingList.h"
#include "KinoSearch/Index/Lexicon.h"
#include "KinoSearch/Index/Posting.h"
#include "KinoSearch/Util/Memory.h"

PostingList*
PList_init(PostingList *self)
{
    ABSTRACT_CLASS_CHECK(self, POSTINGLIST);
    return self;
}


