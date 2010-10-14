#define C_KINO_NOMATCHQUERY
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/NoMatchQuery.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/NoMatchScorer.h"
#include "KinoSearch/Search/Searcher.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/Freezer.h"

NoMatchQuery*
NoMatchQuery_new()
{
    NoMatchQuery *self = (NoMatchQuery*)VTable_Make_Obj(NOMATCHQUERY);
    return NoMatchQuery_init(self);
}

NoMatchQuery*
NoMatchQuery_init(NoMatchQuery *self)
{
    return (NoMatchQuery*)Query_init((Query*)self, 0.0f);
}

bool_t
NoMatchQuery_equals(NoMatchQuery *self, Obj *other)
{
    NoMatchQuery *evil_twin = (NoMatchQuery*)other;
    if (!Obj_Is_A(other, NOMATCHQUERY)) return false;
    if (self->boost != evil_twin->boost) return false;
    return true;
}

CharBuf*
NoMatchQuery_to_string(NoMatchQuery *self)
{
    UNUSED_VAR(self);
    return CB_new_from_trusted_utf8("[NOMATCH]", 9);
}

Compiler*
NoMatchQuery_make_compiler(NoMatchQuery *self, Searcher *searcher, 
                            float boost)
{
    return (Compiler*)NoMatchCompiler_new(self, searcher, boost);
}

/**********************************************************************/

NoMatchCompiler*
NoMatchCompiler_new(NoMatchQuery *parent, Searcher *searcher, 
                     float boost)
{
    NoMatchCompiler *self 
        = (NoMatchCompiler*)VTable_Make_Obj(NOMATCHCOMPILER);
    return NoMatchCompiler_init(self, parent, searcher, boost);
}

NoMatchCompiler*
NoMatchCompiler_init(NoMatchCompiler *self, NoMatchQuery *parent, 
                      Searcher *searcher, float boost)
{
    return (NoMatchCompiler*)Compiler_init((Compiler*)self, 
        (Query*)parent, searcher, NULL, boost);
}

NoMatchCompiler*
NoMatchCompiler_deserialize(NoMatchCompiler *self, InStream *instream)
{
    self = self ? self : (NoMatchCompiler*)VTable_Make_Obj(NOMATCHCOMPILER);
    return (NoMatchCompiler*)Compiler_deserialize((Compiler*)self, instream);
}

Matcher*
NoMatchCompiler_make_matcher(NoMatchCompiler *self, SegReader *reader, 
                             bool_t need_score)
{
    UNUSED_VAR(self);
    UNUSED_VAR(reader);
    UNUSED_VAR(need_score);
    return (Matcher*)NoMatchScorer_new();
}

/* Copyright 2008-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

