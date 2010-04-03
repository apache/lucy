#define C_LUCY_OBJ
#include "XSBind.h"
#include "Lucy/Util/StringHelper.h"

/* Convert a Perl hash into a KS Hash.  Caller takes responsibility for a
 * refcount.
 */
static lucy_Hash*
S_perl_hash_to_lucy_hash(HV *phash);

/* Convert a Perl array into a KS VArray.  Caller takes responsibility for a
 * refcount.
 */
static lucy_VArray*
S_perl_array_to_lucy_array(AV *parray);

/* Convert a VArray to a Perl array.  Caller takes responsibility for a
 * refcount.
 */ 
static SV*
S_lucy_array_to_perl_array(lucy_VArray *varray);

/* Convert a Hash to a Perl hash.  Caller takes responsibility for a
 * refcount.
 */ 
static SV*
S_lucy_hash_to_perl_hash(lucy_Hash *hash);

lucy_Obj*
XSBind_new_blank_obj(SV *either_sv)
{
    lucy_VTable *vtable;

    /* Get a VTable. */
    if (   sv_isobject(either_sv) 
        && sv_derived_from(either_sv, "Lucy::Object::Obj")
    ) {
        /* Use the supplied object's VTable. */
        IV iv_ptr = SvIV(SvRV(either_sv));
        lucy_Obj *self = INT2PTR(lucy_Obj*, iv_ptr);
        vtable = self->vtable;
    }
    else {
        /* Use the supplied class name string to find a VTable. */
        STRLEN len;
        char *ptr = SvPVutf8(either_sv, len);
        lucy_ZombieCharBuf *klass = LUCY_ZCB_WRAP_STR(ptr, len);
        vtable = lucy_VTable_singleton((lucy_CharBuf*)klass, NULL);
    }

    /* Use the VTable to allocate a new blank object of the right size. */
    return Lucy_VTable_Make_Obj(vtable);
}

lucy_Obj*
XSBind_sv_to_lucy_obj(SV *sv, lucy_VTable *vtable, void *allocation)
{
    lucy_Obj *retval = XSBind_maybe_sv_to_lucy_obj(sv, vtable, allocation);
    if (!retval) {
        THROW(LUCY_ERR, "Not a %o", Lucy_VTable_Get_Name(vtable));
    }
    return retval;
}

lucy_Obj*
XSBind_maybe_sv_to_lucy_obj(SV *sv, lucy_VTable *vtable, void *allocation)
{
    lucy_Obj *retval = NULL;
    if (XSBind_sv_defined(sv)) {
        if (   sv_isobject(sv) 
            && sv_derived_from(sv, 
                 (char*)Lucy_CB_Get_Ptr8(Lucy_VTable_Get_Name(vtable)))
        ) {
            /* Unwrap a real Lucy object. */
            IV tmp = SvIV( SvRV(sv) );
            retval = INT2PTR(lucy_Obj*, tmp);
        }
        else if (   allocation &&
                 (  vtable == LUCY_ZOMBIECHARBUF
                 || vtable == LUCY_VIEWCHARBUF
                 || vtable == LUCY_CHARBUF
                 || vtable == LUCY_OBJ)
        ) {
            /* Wrap the string from an ordinary Perl scalar inside a
             * ZombieCharBuf. */
            STRLEN size;
            char *ptr = SvPVutf8(sv, size);
            retval = (lucy_Obj*)lucy_ZCB_wrap_str(allocation, ptr, size);
        }
        else if (SvROK(sv)) {
            /* Attempt to convert Perl hashes and arrays into their Lucy
             * analogues. */
            SV *inner = SvRV(sv);
            if (SvTYPE(inner) == SVt_PVAV && vtable == LUCY_VARRAY) {
                retval = (lucy_Obj*)S_perl_array_to_lucy_array((AV*)inner);
            }
            else if (SvTYPE(inner) == SVt_PVHV && vtable == LUCY_HASH) {
                retval = (lucy_Obj*)S_perl_hash_to_lucy_hash((HV*)inner);
            }

            if(retval) {
                 /* Mortalize the converted object -- which is somewhat
                  * dangerous, but is the only way to avoid requiring that the
                  * caller take responsibility for a refcount. */
                SV *mortal = (SV*)Lucy_Obj_To_Host(retval);
                LUCY_DECREF(retval);
                sv_2mortal(mortal);
            }
        }
    }

    return retval;
}

SV*
XSBind_lucy_to_perl(lucy_Obj *obj)
{
    if (obj == NULL) {
        return newSV(0);
    }
    else if (Lucy_Obj_Is_A(obj, LUCY_CHARBUF)) {
        return XSBind_cb_to_sv((lucy_CharBuf*)obj);
    }
    else if (Lucy_Obj_Is_A(obj, LUCY_BYTEBUF)) {
        return XSBind_bb_to_sv((lucy_ByteBuf*)obj);
    }
    else if (Lucy_Obj_Is_A(obj, LUCY_VARRAY)) {
        return S_lucy_array_to_perl_array((lucy_VArray*)obj);
    }
    else if (Lucy_Obj_Is_A(obj, LUCY_HASH)) {
        return S_lucy_hash_to_perl_hash((lucy_Hash*)obj);
    }
    else if (Lucy_Obj_Is_A(obj, LUCY_FLOATNUM)) {
        return newSVnv(Lucy_Obj_To_F64(obj));
    }
    else if (sizeof(IV) == 8 && Lucy_Obj_Is_A(obj, LUCY_INTNUM)) {
        int64_t num = Lucy_Obj_To_I64(obj);
        return newSViv((IV)num);
    }
    else if (sizeof(IV) == 4 && Lucy_Obj_Is_A(obj, LUCY_INTEGER32)) {
        int32_t num = (int32_t)Lucy_Obj_To_I64(obj);
        return newSViv((IV)num);
    }
    else if (sizeof(IV) == 4 && Lucy_Obj_Is_A(obj, LUCY_INTEGER64)) {
        int64_t num = Lucy_Obj_To_I64(obj);
        return newSVnv((double)num); /* lossy */
    }
    else {
        return (SV*)Lucy_Obj_To_Host(obj);
    }
}

lucy_Obj*
XSBind_perl_to_lucy(SV *sv)
{
    lucy_Obj *retval = NULL;

    if (XSBind_sv_defined(sv)) {
        if (SvROK(sv)) {
            /* Deep conversion of references. */
            SV *inner = SvRV(sv);
            if (SvTYPE(inner) == SVt_PVAV) {
                retval = (lucy_Obj*)S_perl_array_to_lucy_array((AV*)inner);
            }
            else if (SvTYPE(inner) == SVt_PVHV) {
                retval = (lucy_Obj*)S_perl_hash_to_lucy_hash((HV*)inner);
            }
            else if (   sv_isobject(sv) 
                     && sv_derived_from(sv, "Lucy::Object::Obj")
            ) {
                IV tmp = SvIV(inner);
                retval = INT2PTR(lucy_Obj*, tmp);
                (void)LUCY_INCREF(retval);
            }
        }

        /* It's either a plain scalar or a non-Lucy Perl object, so
         * stringify. */
        if (!retval) {
            STRLEN len;
            char *ptr = SvPVutf8(sv, len);
            retval = (lucy_Obj*)lucy_CB_new_from_trusted_utf8(ptr, len);
        }
    }
    else if (sv) {
        /* Deep conversion of raw AVs and HVs. */
        if (SvTYPE(sv) == SVt_PVAV) {
            retval = (lucy_Obj*)S_perl_array_to_lucy_array((AV*)sv);
        }
        else if (SvTYPE(sv) == SVt_PVHV) {
            retval = (lucy_Obj*)S_perl_hash_to_lucy_hash((HV*)sv);
        }
    }

    return retval;
}

SV*
XSBind_bb_to_sv(const lucy_ByteBuf *bb) 
{
    return bb 
        ? newSVpvn(Lucy_BB_Get_Buf(bb), Lucy_BB_Get_Size(bb)) 
        : newSV(0);
}

SV*
XSBind_cb_to_sv(const lucy_CharBuf *cb) 
{
    if (!cb) { 
        return newSV(0);
    }
    else {
        SV *sv = newSVpvn((char*)Lucy_CB_Get_Ptr8(cb), Lucy_CB_Get_Size(cb));
        SvUTF8_on(sv);
        return sv;
    }
}

static lucy_Hash*
S_perl_hash_to_lucy_hash(HV *phash)
{
    uint32_t  num_keys = hv_iterinit(phash);
    lucy_Hash *retval   = lucy_Hash_new(num_keys);

    while (num_keys--) {
        HE *entry = hv_iternext(phash);
        STRLEN key_len;
        /* Copied from Perl 5.10.0 HePV macro, because the HePV macro in
         * earlier versions of Perl triggers a compiler warning. */
        char *key = HeKLEN(entry) == HEf_SVKEY
                  ? SvPV(HeKEY_sv(entry), key_len) 
                  : ((key_len = HeKLEN(entry)), HeKEY(entry));
        SV *value_sv = HeVAL(entry);
        if (!lucy_StrHelp_utf8_valid(key, key_len)) {
            /* Force key to UTF-8. This is kind of a buggy area for Perl, and
             * may result in round-trip weirdness. */
            SV *key_sv = HeSVKEY_force(entry);
            key = SvPVutf8(key_sv, key_len);
        }

        /* Recurse for each value. */
        Lucy_Hash_Store_Str(retval, key, key_len, 
            XSBind_perl_to_lucy(value_sv));
    }

    return retval;
}

static lucy_VArray*
S_perl_array_to_lucy_array(AV *parray)
{
    const uint32_t size = av_len(parray) + 1;
    lucy_VArray *retval = lucy_VA_new(size);
    uint32_t i;

    /* Iterate over array elems. */
    for (i = 0; i < size; i++) {
        SV **elem_sv = av_fetch(parray, i, false);
        if (elem_sv) {
            lucy_Obj *elem = XSBind_perl_to_lucy(*elem_sv);
            if (elem) { Lucy_VA_Store(retval, i, elem); }
        }
    }
    Lucy_VA_Resize(retval, size); /* needed if last elem is NULL */

    return retval;
}

static SV*
S_lucy_array_to_perl_array(lucy_VArray *varray)
{
    AV *perl_array = newAV();
    uint32_t num_elems = Lucy_VA_Get_Size(varray);

    /* Iterate over array elems. */
    if (num_elems) {
        uint32_t i;
        av_fill(perl_array, num_elems - 1);
        for (i = 0; i < num_elems; i++) {
            lucy_Obj *val = Lucy_VA_Fetch(varray, i);
            if (val == NULL) {
                continue;
            }
            else {
                /* Recurse for each value. */
                SV *const val_sv = XSBind_lucy_to_perl(val);
                av_store(perl_array, i, val_sv);
            }
        }
    }

    return newRV_noinc((SV*)perl_array);
}

static SV*
S_lucy_hash_to_perl_hash(lucy_Hash *hash)
{
    HV *perl_hash = newHV();
    SV *key_sv    = newSV(1);
    lucy_CharBuf *key;
    lucy_Obj     *val;

    /* Prepare the SV key. */
    SvPOK_on(key_sv);
    SvUTF8_on(key_sv);

    /* Iterate over key-value pairs. */
    Lucy_Hash_Iter_Init(hash);
    while (Lucy_Hash_Iter_Next(hash, (lucy_Obj**)&key, &val)) {
        /* Recurse for each value. */
        SV *val_sv = XSBind_lucy_to_perl(val);
        if (!Lucy_Obj_Is_A((lucy_Obj*)key, LUCY_CHARBUF)) {
            LUCY_THROW(LUCY_ERR, 
                "Can't convert a key of class %o to a Perl hash key",
                Lucy_Obj_Get_Class_Name((lucy_Obj*)key));
        }
        else {
            STRLEN key_size = Lucy_CB_Get_Size(key);
            char *key_sv_ptr = SvGROW(key_sv, key_size + 1); 
            memcpy(key_sv_ptr, Lucy_CB_Get_Ptr8(key), key_size);
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
    char *package_name = HvNAME(stash);
    size_t size = strlen(package_name);

    /* This code is informed by the following snippet from Perl_sv_bless, from
     * sv.c:
     *
     *     if (Gv_AMG(stash))
     *         SvAMAGIC_on(sv);
     *     else
     *         (void)SvAMAGIC_off(sv);
     *
     * Gv_AMupdate is undocumented.  It is extracted from the Gv_AMG macro,
     * also undocumented, defined in sv.h:
     *
     *     #define Gv_AMG(stash)  (PL_amagic_generation && Gv_AMupdate(stash))
     * 
     * The purpose of the code is to turn on overloading for the class in
     * question.  It seems that as soon as overloading is on for any class,
     * anywhere, that PL_amagic_generation goes positive and stays positive,
     * so that Gv_AMupdate gets called with every bless() invocation.  Since
     * we need overloading for Doc and all its subclasses, we skip the check
     * and just update every time.
     */
    stash = gv_stashpvn((char*)package_name, size, true);
    /* Gv_AMupdate() changed in Perl 5.11 to take a second argument.
     * http://www.mail-archive.com/perl5-changes@perl.org/msg23145.html
     */
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

    /* Retrieve the params hash, which must be a package global. */
    if (params_hash == NULL) {
        THROW(LUCY_ERR, "Can't find hash named %s", params_hash_name);
    }

    /* Verify that our args come in pairs. Bail if there are no args. */
    if (num_stack_elems == start) { return; }
    if ((num_stack_elems - start) % 2 != 0) {
        THROW(LUCY_ERR, "Expecting hash-style params, got odd number of args");
    }

    /* Validate param names. */
    for (i = start; i < num_stack_elems; i += 2) {
        SV *const key_sv = stack[i];
        STRLEN key_len;
        const char *key = SvPV(key_sv, key_len); /* assume ASCII labels */
        if (!hv_exists(params_hash, key, key_len)) {
            THROW(LUCY_ERR, "Invalid parameter: '%s'", key);
        }
    }

    va_start(args, params_hash_name); 
    while (args_left && NULL != (target = va_arg(args, SV**))) {
        char *label = va_arg(args, char*);
        int label_len = va_arg(args, int);

        /* Iterate through stack looking for a label match. Work backwards so
         * that if the label is doubled up we get the last one. */
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

/**
 * Copyright 2009 The Apache Software Foundation
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

