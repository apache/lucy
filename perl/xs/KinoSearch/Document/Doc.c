/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define C_KINO_DOC
#include "xs/XSBind.h"
#include "KinoSearch/Document/Doc.h"
#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Store/InStream.h"
#include "KinoSearch/Store/OutStream.h"
#include "KinoSearch/Util/Memory.h"

kino_Doc*
kino_Doc_init(kino_Doc *self, void *fields, int32_t doc_id)
{
    // Assign. 
    if (fields) {
        if (SvTYPE((SV*)fields) != SVt_PVHV) THROW(KINO_ERR, "Not a hash");
        self->fields = SvREFCNT_inc((SV*)fields);
    }
    else {
        self->fields = newHV();
    }
    self->doc_id = doc_id;

    return self;
}

void
kino_Doc_set_fields(kino_Doc *self, void *fields)
{
    if (self->fields) SvREFCNT_dec((SV*)self->fields);
    self->fields = SvREFCNT_inc((SV*)fields);
}

uint32_t
kino_Doc_get_size(kino_Doc *self)
{
    return self->fields ? HvKEYS((HV*)self->fields) : 0;
}

void
kino_Doc_store(kino_Doc *self, const kino_CharBuf *field, kino_Obj *value)
{
    char   *key      = (char*)Kino_CB_Get_Ptr8(field);
    size_t  key_size = Kino_CB_Get_Size(field);
    SV *key_sv = newSVpvn(key, key_size); 
    SV *val_sv = value == NULL
               ? newSV(0)
               : Kino_Obj_Is_A(value, KINO_CHARBUF) 
               ? XSBind_cb_to_sv((kino_CharBuf*)value)
               : (SV*)Kino_Obj_To_Host(value);
    SvUTF8_on(key_sv);
    hv_store_ent((HV*)self->fields, key_sv, val_sv, 0);
    // TODO: make this a thread-local instead of creating it every time? 
    SvREFCNT_dec(key_sv);
}

void
kino_Doc_serialize(kino_Doc *self, kino_OutStream *outstream)
{
    Kino_OutStream_Write_C32(outstream, self->doc_id);
    kino_Host_callback(self, "serialize_fields", 1, 
        CFISH_ARG_OBJ("outstream", outstream));
}

kino_Doc*
kino_Doc_deserialize(kino_Doc *self, kino_InStream *instream)
{
    int32_t doc_id = (int32_t)Kino_InStream_Read_C32(instream);

    self = self ? self : (kino_Doc*)Kino_VTable_Make_Obj(KINO_DOC);
    kino_Doc_init(self, NULL, doc_id);
    kino_Host_callback(self, "deserialize_fields", 1, 
        CFISH_ARG_OBJ("instream", instream));
    
    return self;
}

kino_Obj*
kino_Doc_extract(kino_Doc *self, kino_CharBuf *field, 
                 kino_ViewCharBuf *target) 
{
    kino_Obj *retval = NULL;
    SV **sv_ptr = hv_fetch((HV*)self->fields, (char*)Kino_CB_Get_Ptr8(field), 
        Kino_CB_Get_Size(field), 0);

    if (sv_ptr && XSBind_sv_defined(*sv_ptr)) {
        SV *const sv = *sv_ptr;
        if (sv_isobject(sv) && sv_derived_from(sv, "KinoSearch::Object::Obj")) {
            IV tmp = SvIV( SvRV(sv) );
            retval = INT2PTR(kino_Obj*, tmp);
        }
        else {
            STRLEN size;
            char *ptr = SvPVutf8(sv, size);
            Kino_ViewCB_Assign_Str(target, ptr, size);
            retval = (kino_Obj*)target;
        }
    }

    return retval;
}
    
void*
kino_Doc_to_host(kino_Doc *self)
{
    kino_Doc_to_host_t super_to_host 
        = (kino_Doc_to_host_t)LUCY_SUPER_METHOD(KINO_DOC, Doc, To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

kino_Hash*
kino_Doc_dump(kino_Doc *self)
{
    kino_Hash *dump = kino_Hash_new(0);
    Kino_Hash_Store_Str(dump, "_class", 6, 
        (kino_Obj*)Kino_CB_Clone(Kino_Doc_Get_Class_Name(self)));
    Kino_Hash_Store_Str(dump, "doc_id", 7, 
        (kino_Obj*)kino_CB_newf("%i32", self->doc_id));
    Kino_Hash_Store_Str(dump, "fields", 6, 
        XSBind_perl_to_cfish((SV*)self->fields));
    return dump;
}

kino_Doc*
kino_Doc_load(kino_Doc *self, kino_Obj *dump)
{
    kino_Hash *source = (kino_Hash*)CFISH_CERTIFY(dump, KINO_HASH);
    kino_CharBuf *class_name = (kino_CharBuf*)CFISH_CERTIFY(
        Kino_Hash_Fetch_Str(source, "_class", 6), KINO_CHARBUF);
    kino_VTable *vtable = kino_VTable_singleton(class_name, NULL);
    kino_Doc *loaded = (kino_Doc*)Kino_VTable_Make_Obj(vtable);
    kino_Obj *doc_id = CFISH_CERTIFY(
        Kino_Hash_Fetch_Str(source, "doc_id", 7), KINO_OBJ);
    kino_Hash *fields = (kino_Hash*)CFISH_CERTIFY(
        Kino_Hash_Fetch_Str(source, "fields", 6), KINO_HASH);
    SV *fields_sv = XSBind_cfish_to_perl((kino_Obj*)fields);
    CHY_UNUSED_VAR(self);

    loaded->doc_id = (int32_t)Kino_Obj_To_I64(doc_id);
    loaded->fields  = SvREFCNT_inc(SvRV(fields_sv));
    SvREFCNT_dec(fields_sv);

    return loaded;
}

chy_bool_t
kino_Doc_equals(kino_Doc *self, kino_Obj *other)
{
    kino_Doc *evil_twin = (kino_Doc*)other;
    HV *my_fields;
    HV *other_fields;
    I32 num_fields;

    if (evil_twin == self)                    { return true;  }
    if (!Kino_Obj_Is_A(other, KINO_DOC))      { return false; }
    if (!self->doc_id == evil_twin->doc_id)   { return false; }
    if (!!self->fields ^ !!evil_twin->fields) { return false; }

    // Verify fields.  Don't allow any deep data structures. 
    my_fields    = (HV*)self->fields;
    other_fields = (HV*)evil_twin->fields;
    if (HvKEYS(my_fields) != HvKEYS(other_fields)) { return false; }
    num_fields = hv_iterinit(my_fields);
    while(num_fields--) {
        HE *my_entry = hv_iternext(my_fields);
        SV *my_val_sv = HeVAL(my_entry);
        STRLEN key_len = HeKLEN(my_entry);
        char *key = HeKEY(my_entry);
        SV **const other_val = hv_fetch(other_fields, key, key_len, 0);
        if (!other_val) { return false; }
        if (!sv_eq(my_val_sv, *other_val)) { return false; }
    }

    return true;
}

void
kino_Doc_destroy(kino_Doc *self)
{
    if (self->fields) SvREFCNT_dec((SV*)self->fields);
    KINO_SUPER_DESTROY(self, KINO_DOC);
}


