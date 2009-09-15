#define C_LUCY_OBJ

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include "Lucy/Object/Obj.h"
#include "Lucy/Object/CharBuf.h"
#include "Lucy/Object/Err.h"
#include "Lucy/Object/VTable.h"
#include "Lucy/Util/Memory.h"

static void
S_lazy_init_host_obj(lucy_Obj *self) 
{
    size_t old_refcount = self->ref.count;

    /* Find the Perl stash entry. */
    lucy_CharBuf *class_name = Lucy_VTable_Get_Name(self->vtable);
    HV *stash = gv_stashpvn((char*)Lucy_CB_Get_Ptr8(class_name),
        Lucy_CB_Get_Size(class_name), TRUE);

    /* In Perl, object SVs are never accessed directly; they are always
     * wrapped within a reference SV.  However, we have no need for the extra
     * level of indirection, so we create the inner object directly and avoid
     * the extra allocation.  If and when we need to send the object back into
     * Perl space, we'll create the RV (in To_Host).
     * 
     * (Note: Some code cribbed from Perl_sv_bless, in sv.c.) */
    SV *inner_obj = newSV(0);
    SvOBJECT_on(inner_obj);
    PL_sv_objcount++;
    SvUPGRADE(inner_obj, SVt_PVMG);
    SvSTASH_set(inner_obj, (HV*)SvREFCNT_inc(stash));
    sv_setiv(inner_obj, PTR2IV(self));

    /* Up till now we've been keeping track of the refcount in
     * self->ref.count.  We're replacing ref.count with ref.host_obj, which
     * will assume responsibility for maintaining the refcount.  ref.host_obj
     * starts off with a refcount of 1, so we need to transfer any refcounts
     * in excess of that. */
    self->ref.host_obj = inner_obj;
    while(old_refcount > 1) {
        SvREFCNT_inc_simple_void_NN(inner_obj);
        old_refcount--;
    }
}

chy_u32_t
lucy_Obj_get_refcount(lucy_Obj *self)
{
    return self->ref.count < 4 
        ? self->ref.count
        : SvREFCNT((SV*)self->ref.host_obj);
}

lucy_Obj*
lucy_Obj_inc_refcount(lucy_Obj *self)
{
    switch (self->ref.count) {
        case 0:
            LUCY_THROW(LUCY_ERR, "Illegal refcount of 0");
            break; /* useless */
        case 1:
        case 2:
            self->ref.count++;
            break;
        case 3:
            S_lazy_init_host_obj(self);
            /* fall through */
        default:
            SvREFCNT_inc_simple_void_NN((SV*)self->ref.host_obj);
    }
    return self;
}

chy_u32_t
lucy_Obj_dec_refcount(lucy_Obj *self)
{
    chy_u32_t modified_refcount = I32_MAX;
    switch (self->ref.count) {
        case 0:
            LUCY_THROW(LUCY_ERR, "Illegal refcount of 0");
            break; /* useless */
        case 1:
            modified_refcount = 0;
            Lucy_Obj_Destroy(self);
            break;
        case 2:
        case 3:
            modified_refcount = --self->ref.count;
            break;
        default:
            modified_refcount = SvREFCNT((SV*)self->ref.host_obj) - 1;
            /* If the SV's refcount falls to 0, DESTROY will be invoked from
             * Perl-space.
             */
            SvREFCNT_dec((SV*)self->ref.host_obj);
    }
    return modified_refcount;
}

void*
lucy_Obj_to_host(lucy_Obj *self)
{
    if (self->ref.count < 4) { S_lazy_init_host_obj(self); }
    return newRV_inc((SV*)self->ref.host_obj);
}

/* Copyright 2009 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

