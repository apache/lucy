#define C_KINO_SPAN
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/Span.h"

Span*
Span_new(int32_t offset, int32_t length, float weight)
{
    Span *self = (Span*)VTable_Make_Obj(SPAN);
    return Span_init(self, offset, length, weight);
}

Span*
Span_init(Span *self, int32_t offset, int32_t length, 
            float weight)
{
    self->offset   = offset;
    self->length   = length;
    self->weight   = weight;
    return self;
}

int32_t
Span_get_offset(Span *self) { return self->offset; }
int32_t
Span_get_length(Span *self) { return self->length; }
float
Span_get_weight(Span *self) { return self->weight; }
void
Span_set_offset(Span *self, int32_t offset) { self->offset = offset; }
void
Span_set_length(Span *self, int32_t length) { self->length = length; }
void
Span_set_weight(Span *self, float weight) { self->weight = weight; }

int32_t
Span_compare_to(Span *self, Obj *other)
{
    Span *competitor = (Span*)CERTIFY(other, SPAN);
    int32_t comparison = self->offset - competitor->offset;
    if (comparison == 0) { comparison = self->length - competitor->length; }
    return comparison;
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

