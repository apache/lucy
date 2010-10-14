#define C_KINO_OBJ
#define C_KINO_LOCKFREEREGISTRY
#include "xs/XSBind.h"

#include "KinoSearch/Object/LockFreeRegistry.h"
#include "KinoSearch/Object/Host.h"

void*
kino_LFReg_to_host(kino_LockFreeRegistry *self)
{
    chy_bool_t first_time = self->ref.count < 4 ? true : false;
    kino_LFReg_to_host_t to_host = (kino_LFReg_to_host_t)
        KINO_SUPER_METHOD(KINO_LOCKFREEREGISTRY, LFReg, To_Host);
    SV *host_obj = (SV*)to_host(self);
    if (first_time) {
        SvSHARE((SV*)self->ref.host_obj);
    }
    return host_obj;
}

/* Copyright 2005-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

