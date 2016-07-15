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

#define CFP_LUCY
#define C_LUCY_DOC
#include "XSBind.h"
#include "Lucy/Document/Doc.h"
#include "Lucy/Store/InStream.h"
#include "Lucy/Store/OutStream.h"
#include "Lucy/Util/Json.h"
#include "Clownfish/Util/Memory.h"

lucy_Doc*
lucy_Doc_init(lucy_Doc *self, void *fields, int32_t doc_id) {
    dTHX;
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
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    if (ivars->fields) { SvREFCNT_dec((SV*)ivars->fields); }
    ivars->fields = SvREFCNT_inc((SV*)fields);
}

uint32_t
LUCY_Doc_Get_Size_IMP(lucy_Doc *self) {
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    return ivars->fields ? HvKEYS((HV*)ivars->fields) : 0;
}

void
LUCY_Doc_Store_IMP(lucy_Doc *self, cfish_String *field, cfish_Obj *value) {
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    const char *key      = CFISH_Str_Get_Ptr8(field);
    size_t      key_size = CFISH_Str_Get_Size(field);
    SV *key_sv = newSVpvn(key, key_size);
    SV *val_sv = XSBind_cfish_to_perl(aTHX_ value);
    SvUTF8_on(key_sv);
    (void)hv_store_ent((HV*)ivars->fields, key_sv, val_sv, 0);
    // TODO: make this a thread-local instead of creating it every time?
    SvREFCNT_dec(key_sv);
}

static SV*
S_nfreeze_fields(pTHX_ lucy_Doc *self) {
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
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    LUCY_OutStream_Write_CU32(outstream, ivars->doc_id);
    SV *frozen = S_nfreeze_fields(aTHX_ self);
    STRLEN len;
    char *buf = SvPV(frozen, len);
    LUCY_OutStream_Write_CU64(outstream, len);
    LUCY_OutStream_Write_Bytes(outstream, buf, len);
    SvREFCNT_dec(frozen);
}

static HV*
S_thaw_fields(pTHX_ lucy_InStream *instream) {
    // Read frozen data into an SV buffer.
    size_t len = (size_t)LUCY_InStream_Read_CU64(instream);
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
    dTHX;
    int32_t doc_id = (int32_t)LUCY_InStream_Read_CU32(instream);
    HV *fields = S_thaw_fields(aTHX_ instream);
    lucy_Doc_init(self, fields, doc_id);
    SvREFCNT_dec(fields);
    return self;
}

cfish_Obj*
LUCY_Doc_Extract_IMP(lucy_Doc *self, cfish_String *field) {
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    cfish_Obj *retval = NULL;
    SV **sv_ptr = hv_fetch((HV*)ivars->fields, CFISH_Str_Get_Ptr8(field),
                           -CFISH_Str_Get_Size(field), 0);

    if (sv_ptr) {
        retval = XSBind_perl_to_cfish_nullable(aTHX_ *sv_ptr, CFISH_OBJ);
    }

    return retval;
}

cfish_Vector*
LUCY_Doc_Field_Names_IMP(lucy_Doc *self) {
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);

    HV           *fields     = (HV*)ivars->fields;
    I32           num_fields = hv_iterinit(fields);
    cfish_Vector *retval     = cfish_Vec_new(num_fields);

    while (num_fields--) {
        HE *entry = hv_iternext(fields);
        STRLEN key_size;
        const char *key = XSBind_hash_key_to_utf8(aTHX_ entry, &key_size);
        cfish_String *key_str = cfish_Str_new_from_trusted_utf8(key, key_size);
        CFISH_Vec_Push(retval, (cfish_Obj*)key_str);
    }

    return retval;
}

cfish_Hash*
LUCY_Doc_Dump_IMP(lucy_Doc *self) {
    dTHX;
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    cfish_Hash *dump = cfish_Hash_new(0);
    CFISH_Hash_Store_Utf8(dump, "_class", 6,
                          (cfish_Obj*)CFISH_Str_Clone(lucy_Doc_get_class_name(self)));
    CFISH_Hash_Store_Utf8(dump, "doc_id", 7,
                          (cfish_Obj*)cfish_Str_newf("%i32", ivars->doc_id));
    SV *fields_sv = newRV_inc((SV*)ivars->fields);
    CFISH_Hash_Store_Utf8(dump, "fields", 6,
                          XSBind_perl_to_cfish(aTHX_ fields_sv, CFISH_HASH));
    SvREFCNT_dec(fields_sv);
    return dump;
}

lucy_Doc*
LUCY_Doc_Load_IMP(lucy_Doc *self, cfish_Obj *dump) {
    dTHX;
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
    SV *fields_sv = XSBind_cfish_to_perl(aTHX_ (cfish_Obj*)fields);
    CFISH_UNUSED_VAR(self);

    lucy_DocIVARS *const loaded_ivars = lucy_Doc_IVARS(loaded);
    loaded_ivars->doc_id = (int32_t)lucy_Json_obj_to_i64(doc_id);
    loaded_ivars->fields  = SvREFCNT_inc(SvRV(fields_sv));
    SvREFCNT_dec(fields_sv);

    return loaded;
}

bool
LUCY_Doc_Equals_IMP(lucy_Doc *self, cfish_Obj *other) {
    if ((lucy_Doc*)other  == self)        { return true;  }
    if (!cfish_Obj_is_a(other, LUCY_DOC)) { return false; }
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    lucy_DocIVARS *const ovars = lucy_Doc_IVARS((lucy_Doc*)other);

    if (!!ivars->doc_id ^ !!ovars->doc_id) { return false; }
    if (!!ivars->fields ^ !!ovars->fields) { return false; }

    // Verify fields.  Don't allow any deep data structures.
    dTHX;
    HV *my_fields    = (HV*)ivars->fields;
    HV *other_fields = (HV*)ovars->fields;
    if (HvKEYS(my_fields) != HvKEYS(other_fields)) { return false; }
    I32 num_fields = hv_iterinit(my_fields);
    while (num_fields--) {
        HE *my_entry = hv_iternext(my_fields);
        SV *my_val_sv = HeVAL(my_entry);
        STRLEN key_len;
        char *key;

        if (HeKLEN(my_entry) == HEf_SVKEY) {
            SV *key_sv = HeKEY_sv(my_entry);
            key = SvPV(key_sv, key_len);
            if (SvUTF8(key_sv)) { key_len = -key_len; }
        }
        else {
            key_len = HeKLEN(my_entry);
            key = key_len ? HeKEY(my_entry) : Nullch;
            if (HeKUTF8(my_entry)) { key_len = -key_len; }
        }

        SV **const other_val = hv_fetch(other_fields, key, key_len, 0);
        if (!other_val) { return false; }
        if (!sv_eq(my_val_sv, *other_val)) { return false; }
    }

    return true;
}

void
LUCY_Doc_Destroy_IMP(lucy_Doc *self) {
    lucy_DocIVARS *const ivars = lucy_Doc_IVARS(self);
    if (ivars->fields) {
        dTHX;
        SvREFCNT_dec((SV*)ivars->fields);
    }
    CFISH_SUPER_DESTROY(self, LUCY_DOC);
}


