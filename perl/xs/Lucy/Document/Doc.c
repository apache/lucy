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

#define C_LUCY_DOC
#include "XSBind.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Clownfish/Util/Memory.h"

lucy_Doc*
lucy_Doc_init(lucy_Doc *self, void *fields, int32_t doc_id) {
    // Assign.
    if (fields) {
        if (SvTYPE((SV*)fields) != SVt_PVHV) { THROW(LUCY_ERR, "Not a hash"); }
        self->fields = SvREFCNT_inc((SV*)fields);
    }
    else {
        self->fields = newHV();
    }
    self->doc_id = doc_id;

    return self;
}

void
lucy_Doc_set_fields(lucy_Doc *self, void *fields) {
    if (self->fields) { SvREFCNT_dec((SV*)self->fields); }
    self->fields = SvREFCNT_inc((SV*)fields);
}

uint32_t
lucy_Doc_get_size(lucy_Doc *self) {
    return self->fields ? HvKEYS((HV*)self->fields) : 0;
}

void
lucy_Doc_store(lucy_Doc *self, const lucy_CharBuf *field, lucy_Obj *value) {
    char   *key      = (char*)Lucy_CB_Get_Ptr8(field);
    size_t  key_size = Lucy_CB_Get_Size(field);
    SV *key_sv = newSVpvn(key, key_size);
    SV *val_sv = value == NULL
                 ? newSV(0)
                 : Lucy_Obj_Is_A(value, LUCY_CHARBUF)
                 ? XSBind_cb_to_sv((lucy_CharBuf*)value)
                 : (SV*)Lucy_Obj_To_Host(value);
    SvUTF8_on(key_sv);
    (void)hv_store_ent((HV*)self->fields, key_sv, val_sv, 0);
    // TODO: make this a thread-local instead of creating it every time?
    SvREFCNT_dec(key_sv);
}

static SV*
S_nfreeze_fields(lucy_Doc *self) {
    dSP;
    ENTER;
    SAVETMPS;
    EXTEND(SP, 1);
    PUSHMARK(SP);
    mPUSHs((SV*)newRV_inc((SV*)self->fields));
    PUTBACK;
    call_pv("Storable::nfreeze", G_SCALAR);
    SPAGAIN;
    SV *frozen = POPs;
    (void)SvREFCNT_inc(frozen);
    PUTBACK;
    FREETMPS;
    LEAVE;
    return frozen;
}

void
lucy_Doc_serialize(lucy_Doc *self, lucy_OutStream *outstream) {
    Lucy_OutStream_Write_C32(outstream, self->doc_id);
    SV *frozen = S_nfreeze_fields(self);
    STRLEN len;
    char *buf = SvPV(frozen, len);
    Lucy_OutStream_Write_C64(outstream, len);
    Lucy_OutStream_Write_Bytes(outstream, buf, len);
    SvREFCNT_dec(frozen);
}

static HV*
S_thaw_fields(lucy_InStream *instream) {
    // Read frozen data into an SV buffer.
    size_t len = (size_t)Lucy_InStream_Read_C64(instream);
    SV *buf_sv = newSV(len + 1);
    SvPOK_on(buf_sv);
    SvCUR_set(buf_sv, len);
    char *buf = SvPVX(buf_sv);
    Lucy_InStream_Read_Bytes(instream, buf, len);

    // Call back to Storable to thaw the frozen hash.
    dSP;
    ENTER;
    SAVETMPS;
    EXTEND(SP, 1);
    PUSHMARK(SP);
    mPUSHs(buf_sv);
    PUTBACK;
    call_pv("Storable::thaw", G_SCALAR);
    SPAGAIN;
    SV *frozen = POPs;
    if (frozen && !SvROK(frozen)) {
        CFISH_THROW(CFISH_ERR, "thaw failed");
    }
    HV *fields = (HV*)SvRV(frozen);
    (void)SvREFCNT_inc((SV*)fields);
    PUTBACK;
    FREETMPS;
    LEAVE;

    return fields;
}

lucy_Doc*
lucy_Doc_deserialize(lucy_Doc *self, lucy_InStream *instream) {
    int32_t doc_id = (int32_t)Lucy_InStream_Read_C32(instream);
    HV *fields = S_thaw_fields(instream);
    lucy_Doc_init(self, fields, doc_id);
    SvREFCNT_dec(fields);
    return self;
}

lucy_Obj*
lucy_Doc_extract(lucy_Doc *self, lucy_CharBuf *field,
                 lucy_ViewCharBuf *target) {
    lucy_Obj *retval = NULL;
    SV **sv_ptr = hv_fetch((HV*)self->fields, (char*)Lucy_CB_Get_Ptr8(field),
                           Lucy_CB_Get_Size(field), 0);

    if (sv_ptr && XSBind_sv_defined(*sv_ptr)) {
        SV *const sv = *sv_ptr;
        if (sv_isobject(sv) && sv_derived_from(sv, "Clownfish::Obj")) {
            IV tmp = SvIV(SvRV(sv));
            retval = INT2PTR(lucy_Obj*, tmp);
        }
        else {
            STRLEN size;
            char *ptr = SvPVutf8(sv, size);
            Lucy_ViewCB_Assign_Str(target, ptr, size);
            retval = (lucy_Obj*)target;
        }
    }

    return retval;
}

void*
lucy_Doc_to_host(lucy_Doc *self) {
    Lucy_Doc_To_Host_t super_to_host
        = CFISH_SUPER_METHOD_PTR(LUCY_DOC, Lucy_Doc_To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

lucy_Hash*
lucy_Doc_dump(lucy_Doc *self) {
    lucy_Hash *dump = lucy_Hash_new(0);
    Lucy_Hash_Store_Str(dump, "_class", 6,
                        (lucy_Obj*)Lucy_CB_Clone(Lucy_Doc_Get_Class_Name(self)));
    Lucy_Hash_Store_Str(dump, "doc_id", 7,
                        (lucy_Obj*)lucy_CB_newf("%i32", self->doc_id));
    Lucy_Hash_Store_Str(dump, "fields", 6,
                        XSBind_perl_to_cfish((SV*)self->fields));
    return dump;
}

lucy_Doc*
lucy_Doc_load(lucy_Doc *self, lucy_Obj *dump) {
    lucy_Hash *source = (lucy_Hash*)CFISH_CERTIFY(dump, LUCY_HASH);
    lucy_CharBuf *class_name = (lucy_CharBuf*)CFISH_CERTIFY(
                                   Lucy_Hash_Fetch_Str(source, "_class", 6),
                                   LUCY_CHARBUF);
    lucy_VTable *vtable = lucy_VTable_singleton(class_name, NULL);
    lucy_Doc *loaded = (lucy_Doc*)Lucy_VTable_Make_Obj(vtable);
    lucy_Obj *doc_id = CFISH_CERTIFY(
                           Lucy_Hash_Fetch_Str(source, "doc_id", 7),
                           LUCY_OBJ);
    lucy_Hash *fields = (lucy_Hash*)CFISH_CERTIFY(
                            Lucy_Hash_Fetch_Str(source, "fields", 6),
                            LUCY_HASH);
    SV *fields_sv = XSBind_cfish_to_perl((lucy_Obj*)fields);
    CHY_UNUSED_VAR(self);

    loaded->doc_id = (int32_t)Lucy_Obj_To_I64(doc_id);
    loaded->fields  = SvREFCNT_inc(SvRV(fields_sv));
    SvREFCNT_dec(fields_sv);

    return loaded;
}

bool
lucy_Doc_equals(lucy_Doc *self, lucy_Obj *other) {
    lucy_Doc *twin = (lucy_Doc*)other;
    HV *my_fields;
    HV *other_fields;
    I32 num_fields;

    if (twin == self)                    { return true;  }
    if (!Lucy_Obj_Is_A(other, LUCY_DOC)) { return false; }
    if (!self->doc_id == twin->doc_id)   { return false; }
    if (!!self->fields ^ !!twin->fields) { return false; }

    // Verify fields.  Don't allow any deep data structures.
    my_fields    = (HV*)self->fields;
    other_fields = (HV*)twin->fields;
    if (HvKEYS(my_fields) != HvKEYS(other_fields)) { return false; }
    num_fields = hv_iterinit(my_fields);
    while (num_fields--) {
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
lucy_Doc_destroy(lucy_Doc *self) {
    if (self->fields) { SvREFCNT_dec((SV*)self->fields); }
    LUCY_SUPER_DESTROY(self, LUCY_DOC);
}


