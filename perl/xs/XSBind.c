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

#define C_KINO_OBJ
#define NEED_newRV_noinc
#include "XSBind.h"
#include "KinoSearch/Util/StringHelper.h"

// Convert a Perl hash into a KS Hash.  Caller takes responsibility for a
// refcount.
static kino_Hash*
S_perl_hash_to_kino_hash(HV *phash);

// Convert a Perl array into a KS VArray.  Caller takes responsibility for a
// refcount.
static kino_VArray*
S_perl_array_to_kino_array(AV *parray);

// Convert a VArray to a Perl array.  Caller takes responsibility for a
// refcount.
static SV*
S_kino_array_to_perl_array(kino_VArray *varray);

// Convert a Hash to a Perl hash.  Caller takes responsibility for a refcount.
static SV*
S_kino_hash_to_perl_hash(kino_Hash *hash);

kino_Obj*
XSBind_new_blank_obj(SV *either_sv)
{
    kino_VTable *vtable;

    // Get a VTable. 
    if (   sv_isobject(either_sv) 
        && sv_derived_from(either_sv, "KinoSearch::Object::Obj")
    ) {
        // Use the supplied object's VTable. 
        IV iv_ptr = SvIV(SvRV(either_sv));
        kino_Obj *self = INT2PTR(kino_Obj*, iv_ptr);
        vtable = self->vtable;
    }
    else {
        // Use the supplied class name string to find a VTable. 
        STRLEN len;
        char *ptr = SvPVutf8(either_sv, len);
        kino_ZombieCharBuf *klass = KINO_ZCB_WRAP_STR(ptr, len);
        vtable = kino_VTable_singleton((kino_CharBuf*)klass, NULL);
    }

    // Use the VTable to allocate a new blank object of the right size. 
    return Kino_VTable_Make_Obj(vtable);
}

kino_Obj*
XSBind_sv_to_kino_obj(SV *sv, kino_VTable *vtable, void *allocation)
{
    kino_Obj *retval = XSBind_maybe_sv_to_kino_obj(sv, vtable, allocation);
    if (!retval) {
        THROW(KINO_ERR, "Not a %o", Kino_VTable_Get_Name(vtable));
    }
    return retval;
}

kino_Obj*
XSBind_maybe_sv_to_kino_obj(SV *sv, kino_VTable *vtable, void *allocation)
{
    kino_Obj *retval = NULL;
    if (XSBind_sv_defined(sv)) {
        if (   sv_isobject(sv) 
            && sv_derived_from(sv, 
                 (char*)Kino_CB_Get_Ptr8(Kino_VTable_Get_Name(vtable)))
        ) {
            // Unwrap a real KinoSearch object. 
            IV tmp = SvIV( SvRV(sv) );
            retval = INT2PTR(kino_Obj*, tmp);
        }
        else if (   allocation &&
                 (  vtable == KINO_ZOMBIECHARBUF
                 || vtable == KINO_VIEWCHARBUF
                 || vtable == KINO_CHARBUF
                 || vtable == KINO_OBJ)
        ) {
            // Wrap the string from an ordinary Perl scalar inside a
            // ZombieCharBuf.
            STRLEN size;
            char *ptr = SvPVutf8(sv, size);
            retval = (kino_Obj*)kino_ZCB_wrap_str(allocation, ptr, size);
        }
        else if (SvROK(sv)) {
            // Attempt to convert Perl hashes and arrays into their KinoSearch
            // analogues.
            SV *inner = SvRV(sv);
            if (SvTYPE(inner) == SVt_PVAV && vtable == KINO_VARRAY) {
                retval = (kino_Obj*)S_perl_array_to_kino_array((AV*)inner);
            }
            else if (SvTYPE(inner) == SVt_PVHV && vtable == KINO_HASH) {
                retval = (kino_Obj*)S_perl_hash_to_kino_hash((HV*)inner);
            }

            if(retval) {
                // Mortalize the converted object -- which is somewhat
                // dangerous, but is the only way to avoid requiring that the
                // caller take responsibility for a refcount.
                SV *mortal = (SV*)Kino_Obj_To_Host(retval);
                KINO_DECREF(retval);
                sv_2mortal(mortal);
            }
        }
    }

    return retval;
}

SV*
XSBind_kino_to_perl(kino_Obj *obj)
{
    if (obj == NULL) {
        return newSV(0);
    }
    else if (Kino_Obj_Is_A(obj, KINO_CHARBUF)) {
        return XSBind_cb_to_sv((kino_CharBuf*)obj);
    }
    else if (Kino_Obj_Is_A(obj, KINO_BYTEBUF)) {
        return XSBind_bb_to_sv((kino_ByteBuf*)obj);
    }
    else if (Kino_Obj_Is_A(obj, KINO_VARRAY)) {
        return S_kino_array_to_perl_array((kino_VArray*)obj);
    }
    else if (Kino_Obj_Is_A(obj, KINO_HASH)) {
        return S_kino_hash_to_perl_hash((kino_Hash*)obj);
    }
    else if (Kino_Obj_Is_A(obj, KINO_FLOATNUM)) {
        return newSVnv(Kino_Obj_To_F64(obj));
    }
    else if (sizeof(IV) == 8 && Kino_Obj_Is_A(obj, KINO_INTNUM)) {
        int64_t num = Kino_Obj_To_I64(obj);
        return newSViv((IV)num);
    }
    else if (sizeof(IV) == 4 && Kino_Obj_Is_A(obj, KINO_INTEGER32)) {
        int32_t num = (int32_t)Kino_Obj_To_I64(obj);
        return newSViv((IV)num);
    }
    else if (sizeof(IV) == 4 && Kino_Obj_Is_A(obj, KINO_INTEGER64)) {
        int64_t num = Kino_Obj_To_I64(obj);
        return newSVnv((double)num); // lossy 
    }
    else {
        return (SV*)Kino_Obj_To_Host(obj);
    }
}

kino_Obj*
XSBind_perl_to_kino(SV *sv)
{
    kino_Obj *retval = NULL;

    if (XSBind_sv_defined(sv)) {
        if (SvROK(sv)) {
            // Deep conversion of references. 
            SV *inner = SvRV(sv);
            if (SvTYPE(inner) == SVt_PVAV) {
                retval = (kino_Obj*)S_perl_array_to_kino_array((AV*)inner);
            }
            else if (SvTYPE(inner) == SVt_PVHV) {
                retval = (kino_Obj*)S_perl_hash_to_kino_hash((HV*)inner);
            }
            else if (   sv_isobject(sv) 
                     && sv_derived_from(sv, "KinoSearch::Object::Obj")
            ) {
                IV tmp = SvIV(inner);
                retval = INT2PTR(kino_Obj*, tmp);
                (void)KINO_INCREF(retval);
            }
        }

        // It's either a plain scalar or a non-KinoSearch Perl object, so
        // stringify.
        if (!retval) {
            STRLEN len;
            char *ptr = SvPVutf8(sv, len);
            retval = (kino_Obj*)kino_CB_new_from_trusted_utf8(ptr, len);
        }
    }
    else if (sv) {
        // Deep conversion of raw AVs and HVs. 
        if (SvTYPE(sv) == SVt_PVAV) {
            retval = (kino_Obj*)S_perl_array_to_kino_array((AV*)sv);
        }
        else if (SvTYPE(sv) == SVt_PVHV) {
            retval = (kino_Obj*)S_perl_hash_to_kino_hash((HV*)sv);
        }
    }

    return retval;
}

SV*
XSBind_bb_to_sv(const kino_ByteBuf *bb) 
{
    return bb 
        ? newSVpvn(Kino_BB_Get_Buf(bb), Kino_BB_Get_Size(bb)) 
        : newSV(0);
}

SV*
XSBind_cb_to_sv(const kino_CharBuf *cb) 
{
    if (!cb) { 
        return newSV(0);
    }
    else {
        SV *sv = newSVpvn((char*)Kino_CB_Get_Ptr8(cb), Kino_CB_Get_Size(cb));
        SvUTF8_on(sv);
        return sv;
    }
}

static kino_Hash*
S_perl_hash_to_kino_hash(HV *phash)
{
    uint32_t  num_keys = hv_iterinit(phash);
    kino_Hash *retval   = kino_Hash_new(num_keys);

    while (num_keys--) {
        char *key;
        STRLEN key_len;
        HE *entry = hv_iternext(phash);
        STRLEN he_key_len = HeKLEN(entry);
        SV *value_sv = HeVAL(entry);

        // Force key to UTF-8 if necessary.
        if (he_key_len == (STRLEN)HEf_SVKEY) {
            SV *key_sv = HeKEY_sv(entry);
            key = SvPVutf8(key_sv, key_len);
        }
        else {
            key = HeKEY(entry);
            key_len = he_key_len;
            if (!kino_StrHelp_utf8_valid(key, key_len)) {
                SV *key_sv = HeSVKEY_force(entry);
                key = SvPVutf8(key_sv, key_len);
            }
        }

        // Recurse for each value. 
        Kino_Hash_Store_Str(retval, key, key_len, 
            XSBind_perl_to_kino(value_sv));
    }

    return retval;
}

static kino_VArray*
S_perl_array_to_kino_array(AV *parray)
{
    const uint32_t size = av_len(parray) + 1;
    kino_VArray *retval = kino_VA_new(size);
    uint32_t i;

    // Iterate over array elems. 
    for (i = 0; i < size; i++) {
        SV **elem_sv = av_fetch(parray, i, false);
        if (elem_sv) {
            kino_Obj *elem = XSBind_perl_to_kino(*elem_sv);
            if (elem) { Kino_VA_Store(retval, i, elem); }
        }
    }
    Kino_VA_Resize(retval, size); // needed if last elem is NULL 

    return retval;
}

static SV*
S_kino_array_to_perl_array(kino_VArray *varray)
{
    AV *perl_array = newAV();
    uint32_t num_elems = Kino_VA_Get_Size(varray);

    // Iterate over array elems. 
    if (num_elems) {
        uint32_t i;
        av_fill(perl_array, num_elems - 1);
        for (i = 0; i < num_elems; i++) {
            kino_Obj *val = Kino_VA_Fetch(varray, i);
            if (val == NULL) {
                continue;
            }
            else {
                // Recurse for each value. 
                SV *const val_sv = XSBind_kino_to_perl(val);
                av_store(perl_array, i, val_sv);
            }
        }
    }

    return newRV_noinc((SV*)perl_array);
}

static SV*
S_kino_hash_to_perl_hash(kino_Hash *hash)
{
    HV *perl_hash = newHV();
    SV *key_sv    = newSV(1);
    kino_CharBuf *key;
    kino_Obj     *val;

    // Prepare the SV key. 
    SvPOK_on(key_sv);
    SvUTF8_on(key_sv);

    // Iterate over key-value pairs. 
    Kino_Hash_Iterate(hash);
    while (Kino_Hash_Next(hash, (kino_Obj**)&key, &val)) {
        // Recurse for each value. 
        SV *val_sv = XSBind_kino_to_perl(val);
        if (!Kino_Obj_Is_A((kino_Obj*)key, KINO_CHARBUF)) {
            KINO_THROW(KINO_ERR, 
                "Can't convert a key of class %o to a Perl hash key",
                Kino_Obj_Get_Class_Name((kino_Obj*)key));
        }
        else {
            STRLEN key_size = Kino_CB_Get_Size(key);
            char *key_sv_ptr = SvGROW(key_sv, key_size + 1); 
            memcpy(key_sv_ptr, Kino_CB_Get_Ptr8(key), key_size);
            SvCUR_set(key_sv, key_size);
            *SvEND(key_sv) = '\0';
            hv_store_ent(perl_hash, key_sv, val_sv, 0);
        }
    }
    SvREFCNT_dec(key_sv);

    return newRV_noinc((SV*)perl_hash);
}

void
XSBind_enable_overload(void *pobj)
{
    SV *perl_obj = (SV*)pobj;
    HV *stash = SvSTASH(SvRV(perl_obj));
#if (PERL_VERSION > 10)
    Gv_AMupdate(stash, false);
#else
    Gv_AMupdate(stash);
#endif
    SvAMAGIC_on(perl_obj);
}

void
XSBind_allot_params(SV** stack, int32_t start, int32_t num_stack_elems, 
                    char* params_hash_name, ...)
{
    va_list args;
    HV *params_hash = get_hv(params_hash_name, 0);
    SV **target;
    int32_t i;
    int32_t args_left = (num_stack_elems - start) / 2;

    // Retrieve the params hash, which must be a package global. 
    if (params_hash == NULL) {
        THROW(KINO_ERR, "Can't find hash named %s", params_hash_name);
    }

    // Verify that our args come in pairs. Bail if there are no args. 
    if (num_stack_elems == start) { return; }
    if ((num_stack_elems - start) % 2 != 0) {
        THROW(KINO_ERR, "Expecting hash-style params, got odd number of args");
    }

    // Validate param names. 
    for (i = start; i < num_stack_elems; i += 2) {
        SV *const key_sv = stack[i];
        STRLEN key_len;
        const char *key = SvPV(key_sv, key_len); // assume ASCII labels 
        if (!hv_exists(params_hash, key, key_len)) {
            THROW(KINO_ERR, "Invalid parameter: '%s'", key);
        }
    }

    va_start(args, params_hash_name); 
    while (args_left && NULL != (target = va_arg(args, SV**))) {
        char *label = va_arg(args, char*);
        int label_len = va_arg(args, int);

        // Iterate through stack looking for a label match. Work backwards so
        // that if the label is doubled up we get the last one.
        for (i = num_stack_elems; i >= start + 2; i -= 2) {
            int32_t tick = i - 2;
            SV *const key_sv = stack[tick];
            if (SvCUR(key_sv) == (STRLEN)label_len) {
                if (memcmp(SvPVX(key_sv), label, label_len) == 0) {
                    *target = stack[tick + 1];
                    args_left--;
                    break;
                }
            }
        }
    }
    va_end(args);
}


