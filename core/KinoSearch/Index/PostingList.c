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

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

