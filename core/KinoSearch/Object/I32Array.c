#define C_KINO_I32ARRAY
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Object/I32Array.h"

I32Array*
I32Arr_new(int32_t *ints, uint32_t size) 
{
    I32Array *self = (I32Array*)VTable_Make_Obj(I32ARRAY);
    int32_t *ints_copy = (int32_t*)MALLOCATE(size * sizeof(int32_t));
    memcpy(ints_copy, ints, size * sizeof(int32_t));
    return I32Arr_init(self, ints_copy, size);
}

I32Array*
I32Arr_new_steal(int32_t *ints, uint32_t size) 
{
    I32Array *self = (I32Array*)VTable_Make_Obj(I32ARRAY);
    return I32Arr_init(self, ints, size);
}

I32Array*
I32Arr_init(I32Array *self, int32_t *ints, uint32_t size) 
{
    self->ints = ints;
    self->size = size;
    return self;
}

void
I32Arr_destroy(I32Array *self)
{
    FREEMEM(self->ints);
    SUPER_DESTROY(self, I32ARRAY);
}

int32_t 
I32Arr_get(I32Array *self, uint32_t tick)
{
    if (tick >= self->size) {
        THROW(ERR, "Out of bounds: %u32 >= %u32", tick, self->size);
    }
    return self->ints[tick];
}

uint32_t
I32Arr_get_size(I32Array *self) { return self->size; }

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

