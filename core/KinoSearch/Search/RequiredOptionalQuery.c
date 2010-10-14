#define C_KINO_REQUIREDOPTIONALQUERY
#define C_KINO_REQUIREDOPTIONALCOMPILER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/RequiredOptionalQuery.h"
#include "KinoSearch/Index/SegReader.h"
#include "KinoSearch/Index/Similarity.h"
#include "KinoSearch/Plan/Schema.h"
#include "KinoSearch/Search/RequiredOptionalScorer.h"
#include "KinoSearch/Search/Searcher.h"

RequiredOptionalQuery*
ReqOptQuery_new(Query *required_query, Query *optional_query)
{
    RequiredOptionalQuery *self 
        = (RequiredOptionalQuery*)VTable_Make_Obj(REQUIREDOPTIONALQUERY);
    return ReqOptQuery_init(self, required_query, optional_query);
}

RequiredOptionalQuery*
ReqOptQuery_init(RequiredOptionalQuery *self, Query *required_query, 
                 Query *optional_query)
{
    PolyQuery_init((PolyQuery*)self, NULL);
    VA_Push(self->children, INCREF(required_query));
    VA_Push(self->children, INCREF(optional_query));
    return self;
}

Query*
ReqOptQuery_get_required_query(RequiredOptionalQuery *self) 
{ 
    return (Query*)VA_Fetch(self->children, 0);
}

void
ReqOptQuery_set_required_query(RequiredOptionalQuery *self, 
                               Query *required_query)
{
    VA_Store(self->children, 0, INCREF(required_query));
}

Query*
ReqOptQuery_get_optional_query(RequiredOptionalQuery *self) 
{ 
    return (Query*)VA_Fetch(self->children, 1);
}

void
ReqOptQuery_set_optional_query(RequiredOptionalQuery *self, 
                               Query *optional_query)
{
    VA_Store(self->children, 1, INCREF(optional_query));
}

CharBuf*
ReqOptQuery_to_string(RequiredOptionalQuery *self)
{
    CharBuf *req_string = Obj_To_String(VA_Fetch(self->children, 0));
    CharBuf *opt_string = Obj_To_String(VA_Fetch(self->children, 1));
    CharBuf *retval = CB_newf("(+%o %o)", req_string, opt_string);
    DECREF(opt_string);
    DECREF(req_string);
    return retval;
}

bool_t
ReqOptQuery_equals(RequiredOptionalQuery *self, Obj *other)
{
    if ((RequiredOptionalQuery*)other == self)   { return true;  }
    if (!Obj_Is_A(other, REQUIREDOPTIONALQUERY)) { return false; }
    return PolyQuery_equals((PolyQuery*)self, other);
}

Compiler*
ReqOptQuery_make_compiler(RequiredOptionalQuery *self, Searcher *searcher,
                          float boost)
{
    return (Compiler*)ReqOptCompiler_new(self, searcher, boost);
}

/**********************************************************************/

RequiredOptionalCompiler*
ReqOptCompiler_new(RequiredOptionalQuery *parent, Searcher *searcher,
                   float boost)
{
    RequiredOptionalCompiler *self = (RequiredOptionalCompiler*)
        VTable_Make_Obj(REQUIREDOPTIONALCOMPILER);
    return ReqOptCompiler_init(self, parent, searcher, boost);
}

RequiredOptionalCompiler*
ReqOptCompiler_init(RequiredOptionalCompiler *self,
                    RequiredOptionalQuery *parent, 
                    Searcher *searcher, float boost)
{
    PolyCompiler_init((PolyCompiler*)self, (PolyQuery*)parent, searcher, 
        boost);
    ReqOptCompiler_Normalize(self);
    return self;
}

Matcher*
ReqOptCompiler_make_matcher(RequiredOptionalCompiler *self, SegReader *reader,
                            bool_t need_score)
{
    Schema     *schema       = SegReader_Get_Schema(reader);
    Similarity *sim          = Schema_Get_Similarity(schema);
    Compiler   *req_compiler = (Compiler*)VA_Fetch(self->children, 0);
    Compiler   *opt_compiler = (Compiler*)VA_Fetch(self->children, 1);
    Matcher *req_matcher 
        = Compiler_Make_Matcher(req_compiler, reader, need_score);
    Matcher *opt_matcher 
        = Compiler_Make_Matcher(opt_compiler, reader, need_score);

    if (req_matcher == NULL) {
        // No required matcher, ergo no matches possible. 
        DECREF(opt_matcher);
        return NULL;
    }
    else if (opt_matcher == NULL) {
        return req_matcher;
    }
    else {
        Matcher *retval 
            = (Matcher*)ReqOptScorer_new(sim, req_matcher, opt_matcher);
        DECREF(opt_matcher);
        DECREF(req_matcher);
        return retval;
    }
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

