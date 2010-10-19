#define C_KINO_OBJ
#define C_KINO_VTABLE
#include "xs/XSBind.h"

#include "KinoSearch/Object/VTable.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Util/Memory.h"

kino_Obj*
kino_VTable_foster_obj(kino_VTable *self, void *host_obj)
{
    kino_Obj *obj 
        = (kino_Obj*)kino_Memory_wrapped_calloc(self->obj_alloc_size, 1);
    SV *inner_obj = SvRV((SV*)host_obj);
    obj->vtable = self;
    sv_setiv(inner_obj, PTR2IV(obj));
    obj->ref.host_obj = inner_obj;
    return obj;
}

void
kino_VTable_register_with_host(kino_VTable *singleton, kino_VTable *parent)
{
    // Register class with host. 
    kino_Host_callback(KINO_VTABLE, "_register", 2, 
        KINO_ARG_OBJ("singleton", singleton), KINO_ARG_OBJ("parent", parent));
}

kino_VArray*
kino_VTable_novel_host_methods(const kino_CharBuf *class_name)
{
    return (kino_VArray*)kino_Host_callback_obj(KINO_VTABLE, 
        "novel_host_methods", 1, KINO_ARG_STR("class_name", class_name));
}

kino_CharBuf*
kino_VTable_find_parent_class(const kino_CharBuf *class_name)
{
    return kino_Host_callback_str(KINO_VTABLE, "find_parent_class", 1, 
        KINO_ARG_STR("class_name", class_name));
}

void*
kino_VTable_to_host(kino_VTable *self)
{
    chy_bool_t first_time = self->ref.count < 4 ? true : false;
    kino_VTable_to_host_t to_host = (kino_VTable_to_host_t)
        KINO_SUPER_METHOD(KINO_VTABLE, VTable, To_Host);
    SV *host_obj = (SV*)to_host(self);
    if (first_time) {
        SvSHARE((SV*)self->ref.host_obj);
    }
    return host_obj;
}


