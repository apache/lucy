#define C_KINO_CASEFOLDER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Analysis/CaseFolder.h"
#include "KinoSearch/Analysis/Token.h"
#include "KinoSearch/Analysis/Inversion.h"

CaseFolder*
CaseFolder_new()
{
    CaseFolder *self = (CaseFolder*)VTable_Make_Obj(CASEFOLDER);
    return CaseFolder_init(self);
}

CaseFolder*
CaseFolder_init(CaseFolder *self)
{
    Analyzer_init((Analyzer*)self);
    self->work_buf = BB_new(0);
    return self;
}

void
CaseFolder_destroy(CaseFolder *self)
{
    DECREF(self->work_buf);
    SUPER_DESTROY(self, CASEFOLDER);
}

bool_t
CaseFolder_equals(CaseFolder *self, Obj *other)
{
    CaseFolder *const evil_twin = (CaseFolder*)other;
    if (evil_twin == self) return true;
    UNUSED_VAR(self);
    if (!Obj_Is_A(other, CASEFOLDER)) return false;
    return true;
}

Hash*
CaseFolder_dump(CaseFolder *self)
{
    CaseFolder_dump_t super_dump 
        = (CaseFolder_dump_t)SUPER_METHOD(CASEFOLDER, CaseFolder, Dump);
    return super_dump(self);
}

CaseFolder*
CaseFolder_load(CaseFolder *self, Obj *dump)
{
    CaseFolder_load_t super_load 
        = (CaseFolder_load_t)SUPER_METHOD(CASEFOLDER, CaseFolder, Load);
    CaseFolder *loaded = super_load(self, dump);
    return CaseFolder_init(loaded);
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

