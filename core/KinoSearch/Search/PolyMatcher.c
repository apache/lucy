#define C_KINO_POLYMATCHER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/PolyMatcher.h"
#include "KinoSearch/Index/Similarity.h"

PolyMatcher*
PolyMatcher_new(VArray *children, Similarity *sim) 
{
    PolyMatcher *self = (PolyMatcher*)VTable_Make_Obj(POLYMATCHER);
    return PolyMatcher_init(self, children, sim);
}

PolyMatcher*
PolyMatcher_init(PolyMatcher *self, VArray *children, Similarity *similarity) 
{
    uint32_t i;

    Matcher_init((Matcher*)self);
    self->num_kids = VA_Get_Size(children);
    self->sim      = (Similarity*)INCREF(similarity);
    self->children = (VArray*)INCREF(children);
    self->coord_factors = (float*)MALLOCATE((self->num_kids + 1) * sizeof(float));
    for (i = 0; i <= self->num_kids; i++) {
        self->coord_factors[i] = similarity
                               ? Sim_Coord(similarity, i, self->num_kids) 
                               : 1.0f;
    }
    return self;
}

void
PolyMatcher_destroy(PolyMatcher *self) 
{
    DECREF(self->children);
    DECREF(self->sim);
    FREEMEM(self->coord_factors);
    SUPER_DESTROY(self, POLYMATCHER);
}

/* Copyright 2008-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

