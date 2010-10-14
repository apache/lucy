#define C_KINO_BITVECMATCHER
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Search/BitVecMatcher.h"

BitVecMatcher*
BitVecMatcher_new(BitVector *bit_vector)
{
    BitVecMatcher *self = (BitVecMatcher*)VTable_Make_Obj(BITVECMATCHER);
    return BitVecMatcher_init(self, bit_vector);
}

BitVecMatcher*
BitVecMatcher_init(BitVecMatcher *self, BitVector *bit_vector)
{
    Matcher_init((Matcher*)self);
    self->bit_vec = (BitVector*)INCREF(bit_vector);
    self->doc_id = 0;
    return self;
}

void
BitVecMatcher_destroy(BitVecMatcher *self)
{
    DECREF(self->bit_vec);
    SUPER_DESTROY(self, BITVECMATCHER);
}

int32_t
BitVecMatcher_next(BitVecMatcher *self)
{
    self->doc_id = BitVec_Next_Hit(self->bit_vec, self->doc_id + 1);
    return self->doc_id == -1 ? 0 : self->doc_id;
}

int32_t
BitVecMatcher_advance(BitVecMatcher *self, int32_t target) 
{
    self->doc_id = BitVec_Next_Hit(self->bit_vec, target);
    return self->doc_id == -1 ? 0 : self->doc_id;
}

int32_t
BitVecMatcher_get_doc_id(BitVecMatcher *self) { return self->doc_id; }

/* Copyright 2007-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

