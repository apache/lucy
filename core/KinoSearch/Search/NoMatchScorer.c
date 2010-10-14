#define C_KINO_NOMATCHSCORER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/NoMatchScorer.h"
#include "KinoSearch/Index/IndexReader.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"

NoMatchScorer*
NoMatchScorer_new()
{
    NoMatchScorer *self = (NoMatchScorer*)VTable_Make_Obj(NOMATCHSCORER);
    return NoMatchScorer_init(self);
}

NoMatchScorer*
NoMatchScorer_init(NoMatchScorer *self)
{
    return (NoMatchScorer*)Matcher_init((Matcher*)self);
}   

int32_t
NoMatchScorer_next(NoMatchScorer* self) 
{
    UNUSED_VAR(self);
    return 0;
}

int32_t
NoMatchScorer_advance(NoMatchScorer* self, int32_t target) 
{
    UNUSED_VAR(self);
    UNUSED_VAR(target);
    return 0;
}

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

