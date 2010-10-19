#define C_KINO_OBJ

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "ppport.h"

#include "KinoSearch/Object/Obj.h"
#include "KinoSearch/Object/CharBuf.h"
#include "KinoSearch/Object/Err.h"
#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Util/Memory.h"

static void
S_lazy_init_host_obj(kino_Obj *self) 
{
    SV *inner_obj = newSV(0);
    SvOBJECT_on(inner_obj);
    PL_sv_objcount++;
    SvUPGRADE(inner_obj, SVt_PVMG);
    sv_setiv(inner_obj, PTR2IV(self));

    // Connect class association.
    kino_CharBuf *class_name = Kino_VTable_Get_Name(self->vtable);
    HV *stash = gv_stashpvn((char*)Kino_CB_Get_Ptr8(class_name),
        Kino_CB_Get_Size(class_name), TRUE);
    SvSTASH_set(inner_obj, (HV*)SvREFCNT_inc(stash));

    /* Up till now we've been keeping track of the refcount in
     * self->ref.count.  We're replacing ref.count with ref.host_obj, which
     * will assume responsibility for maintaining the refcount.  ref.host_obj
     * starts off with a refcount of 1, so we need to transfer any refcounts
     * in excess of that. */
    size_t old_refcount = self->ref.count;
    self->ref.host_obj = inner_obj;
    while (old_refcount > 1) {
        SvREFCNT_inc_simple_void_NN(inner_obj);
        old_refcount--;
    }
}

uint32_t
kino_Obj_get_refcount(kino_Obj *self)
{
    return self->ref.count < 4 
        ? self->ref.count
        : SvREFCNT((SV*)self->ref.host_obj);
}

kino_Obj*
kino_Obj_inc_refcount(kino_Obj *self)
{
    switch (self->ref.count) {
        case 0:
            KINO_THROW(KINO_ERR, "Illegal refcount of 0");
            break; // useless 
        case 1:
        case 2:
            self->ref.count++;
            break;
        case 3:
            S_lazy_init_host_obj(self);
            // fall through 
        default:
            SvREFCNT_inc_simple_void_NN((SV*)self->ref.host_obj);
    }
    return self;
}

uint32_t
kino_Obj_dec_refcount(kino_Obj *self)
{
    uint32_t modified_refcount = I32_MAX;
    switch (self->ref.count) {
        case 0:
            KINO_THROW(KINO_ERR, "Illegal refcount of 0");
            break; // useless 
        case 1:
            modified_refcount = 0;
            Kino_Obj_Destroy(self);
            break;
        case 2:
        case 3:
            modified_refcount = --self->ref.count;
            break;
        default:
            modified_refcount = SvREFCNT((SV*)self->ref.host_obj) - 1;
            // If the SV's refcount falls to 0, DESTROY will be invoked from
            // Perl-space.
            SvREFCNT_dec((SV*)self->ref.host_obj);
    }
    return modified_refcount;
}

void*
kino_Obj_to_host(kino_Obj *self)
{
    if (self->ref.count < 4) { S_lazy_init_host_obj(self); }
    return newRV_inc((SV*)self->ref.host_obj);
}


