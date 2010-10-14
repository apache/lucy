#define C_KINO_BITVECDELDOCS
#include "KinoSearch/Util/ToolSet.h"

#include "KinoSearch/Index/BitVecDelDocs.h"
#include "KinoSearch/Store/Folder.h"
#include "KinoSearch/Store/InStream.h"

BitVecDelDocs*
BitVecDelDocs_new(Folder *folder, const CharBuf *filename)
{
    BitVecDelDocs *self = (BitVecDelDocs*)VTable_Make_Obj(BITVECDELDOCS);
    return BitVecDelDocs_init(self, folder, filename);
}

BitVecDelDocs*
BitVecDelDocs_init(BitVecDelDocs *self, Folder *folder, 
                   const CharBuf *filename)
{
    int32_t len;

    BitVec_init((BitVector*)self, 0);
    self->filename = CB_Clone(filename);
    self->instream = Folder_Open_In(folder, filename);
    if (!self->instream) { 
        Err *error = (Err*)INCREF(Err_get_error());
        DECREF(self);
        RETHROW(error);
    }
    len            = (int32_t)InStream_Length(self->instream);
    self->bits     = (uint8_t*)InStream_Buf(self->instream, len);
    self->cap      = (uint32_t)(len * 8);
    return self;
}

void
BitVecDelDocs_destroy(BitVecDelDocs *self)
{
    DECREF(self->filename);
    if (self->instream) {
        InStream_Close(self->instream);
        DECREF(self->instream);
    }
    self->bits = NULL;
    SUPER_DESTROY(self, BITVECDELDOCS);
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

