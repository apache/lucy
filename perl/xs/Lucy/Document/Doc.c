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
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    // Assign.
    if (fields) {
        if (SvTYPE((SV*)fields) != SVt_PVHV) { THROW(CFISH_ERR, "Not a hash"); }
        ivars->fields = SvREFCNT_inc((SV*)fields);
    }
    else {
        ivars->fields = newHV();
    }
    ivars->doc_id = doc_id;

    return self;
}

void
LUCY_Doc_Set_Fields_IMP(lucy_Doc *self, void *fields) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    if (ivars->fields) { SvREFCNT_dec((SV*)ivars->fields); }
    ivars->fields = SvREFCNT_inc((SV*)fields);
}

uint32_t
LUCY_Doc_Get_Size_IMP(lucy_Doc *self) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    return ivars->fields ? HvKEYS((HV*)ivars->fields) : 0;
}

void
LUCY_Doc_Store_IMP(lucy_Doc *self, cfish_String *field, cfish_Obj *value) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    const char *key      = CFISH_Str_Get_Ptr8(field);
    size_t      key_size = CFISH_Str_Get_Size(field);
    SV *key_sv = newSVpvn(key, key_size);
    SV *val_sv = value == NULL
                 ? newSV(0)
                 : CFISH_Obj_Is_A(value, CFISH_STRING)
                 ? XSBind_str_to_sv((cfish_String*)value)
                 : (SV*)CFISH_Obj_To_Host(value);
    SvUTF8_on(key_sv);
    (void)hv_store_ent((HV*)ivars->fields, key_sv, val_sv, 0);
    // TODO: make this a thread-local instead of creating it every time?
    SvREFCNT_dec(key_sv);
}

static SV*
S_nfreeze_fields(lucy_Doc *self) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    dSP;
    ENTER;
    SAVETMPS;
    EXTEND(SP, 1);
    PUSHMARK(SP);
    mPUSHs((SV*)newRV_inc((SV*)ivars->fields));
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
LUCY_Doc_Serialize_IMP(lucy_Doc *self, lucy_OutStream *outstream) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    LUCY_OutStream_Write_C32(outstream, ivars->doc_id);
    SV *frozen = S_nfreeze_fields(self);
    STRLEN len;
    char *buf = SvPV(frozen, len);
    LUCY_OutStream_Write_C64(outstream, len);
    LUCY_OutStream_Write_Bytes(outstream, buf, len);
    SvREFCNT_dec(frozen);
}

static HV*
S_thaw_fields(lucy_InStream *instream) {
    // Read frozen data into an SV buffer.
    size_t len = (size_t)LUCY_InStream_Read_C64(instream);
    SV *buf_sv = newSV(len + 1);
    SvPOK_on(buf_sv);
    SvCUR_set(buf_sv, len);
    char *buf = SvPVX(buf_sv);
    LUCY_InStream_Read_Bytes(instream, buf, len);

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
LUCY_Doc_Deserialize_IMP(lucy_Doc *self, lucy_InStream *instream) {
    int32_t doc_id = (int32_t)LUCY_InStream_Read_C32(instream);
    HV *fields = S_thaw_fields(instream);
    lucy_Doc_init(self, fields, doc_id);
    SvREFCNT_dec(fields);
    return self;
}

cfish_Obj*
LUCY_Doc_Extract_IMP(lucy_Doc *self, cfish_String *field) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    cfish_Obj *retval = NULL;
    SV **sv_ptr = hv_fetch((HV*)ivars->fields, CFISH_Str_Get_Ptr8(field),
                           CFISH_Str_Get_Size(field), 0);

    if (sv_ptr && XSBind_sv_defined(*sv_ptr)) {
        SV *const sv = *sv_ptr;
        if (sv_isobject(sv) && sv_derived_from(sv, "Clownfish::Obj")) {
            IV tmp = SvIV(SvRV(sv));
            retval = CFISH_INCREF(INT2PTR(cfish_Obj*, tmp));
        }
        else {
            STRLEN size;
            char *ptr = SvPVutf8(sv, size);
            retval = (cfish_Obj*)cfish_Str_new_wrap_trusted_utf8(ptr, size);
        }
    }

    return retval;
}

void*
LUCY_Doc_To_Host_IMP(lucy_Doc *self) {
    LUCY_Doc_To_Host_t super_to_host
        = CFISH_SUPER_METHOD_PTR(LUCY_DOC, LUCY_Doc_To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

cfish_Hash*
LUCY_Doc_Dump_IMP(lucy_Doc *self) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    cfish_Hash *dump = cfish_Hash_new(0);
    CFISH_Hash_Store_Utf8(dump, "_class", 6,
                          (cfish_Obj*)CFISH_Str_Clone(LUCY_Doc_Get_Class_Name(self)));
    CFISH_Hash_Store_Utf8(dump, "doc_id", 7,
                          (cfish_Obj*)cfish_Str_newf("%i32", ivars->doc_id));
    CFISH_Hash_Store_Utf8(dump, "fields", 6,
                          XSBind_perl_to_cfish((SV*)ivars->fields));
    return dump;
}

lucy_Doc*
LUCY_Doc_Load_IMP(lucy_Doc *self, cfish_Obj *dump) {
    cfish_Hash *source = (cfish_Hash*)CFISH_CERTIFY(dump, CFISH_HASH);
    cfish_String *class_name = (cfish_String*)CFISH_CERTIFY(
                                   CFISH_Hash_Fetch_Utf8(source, "_class", 6),
                                   CFISH_STRING);
    cfish_Class *klass = cfish_Class_singleton(class_name, NULL);
    lucy_Doc *loaded = (lucy_Doc*)CFISH_Class_Make_Obj(klass);
    cfish_Obj *doc_id = CFISH_CERTIFY(
                           CFISH_Hash_Fetch_Utf8(source, "doc_id", 7),
                           CFISH_OBJ);
    cfish_Hash *fields = (cfish_Hash*)CFISH_CERTIFY(
                            CFISH_Hash_Fetch_Utf8(source, "fields", 6),
                            CFISH_HASH);
    SV *fields_sv = XSBind_cfish_to_perl((cfish_Obj*)fields);
    CFISH_UNUSED_VAR(self);

    lucy_DocIVARS *const loaded_ivars = lucy_Doc_IVARS(loaded);
    loaded_ivars->doc_id = (int32_t)CFISH_Obj_To_I64(doc_id);
    loaded_ivars->fields  = SvREFCNT_inc(SvRV(fields_sv));
    SvREFCNT_dec(fields_sv);

    return loaded;
}

bool
LUCY_Doc_Equals_IMP(lucy_Doc *self, cfish_Obj *other) {
    if ((lucy_Doc*)other  == self)        { return true;  }
    if (!CFISH_Obj_Is_A(other, LUCY_DOC)) { return false; }
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    lucy_DocIVARS *const ovars = lucy_Doc_IVARS((lucy_Doc*)other);

    if (!ivars->doc_id == ovars->doc_id)   { return false; }
    if (!!ivars->fields ^ !!ovars->fields) { return false; }

    // Verify fields.  Don't allow any deep data structures.
    HV *my_fields    = (HV*)ivars->fields;
    HV *other_fields = (HV*)ovars->fields;
    if (HvKEYS(my_fields) != HvKEYS(other_fields)) { return false; }
    I32 num_fields = hv_iterinit(my_fields);
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
LUCY_Doc_Destroy_IMP(lucy_Doc *self) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    if (ivars->fields) { SvREFCNT_dec((SV*)ivars->fields); }
    CFISH_SUPER_DESTROY(self, LUCY_DOC);
}


