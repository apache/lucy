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

#include "XSBind.h"
#include "Clownfish/Host.h"

// Anonymous XSUB helper for Err#trap().  It wraps the supplied C function
// so that it can be run inside a Perl eval block.
static SV *attempt_xsub = NULL;

XS(lucy_Err_attempt_via_xs) {
    dXSARGS;
    CHY_UNUSED_VAR(cv);
    SP -= items;
    if (items != 2) {
        CFISH_THROW(CFISH_ERR, "Usage: $sub->(routine, context)");
    };
    IV routine_iv = SvIV(ST(0));
    IV context_iv = SvIV(ST(1));
    Cfish_Err_Attempt_t routine = INT2PTR(Cfish_Err_Attempt_t, routine_iv);
    void *context               = INT2PTR(void*, context_iv);
    routine(context);
    XSRETURN(0);
}

void
lucy_Err_init_class(void) {
    char *file = (char*)__FILE__;
    attempt_xsub = (SV*)newXS(NULL, lucy_Err_attempt_via_xs, file);
}

lucy_Err*
lucy_Err_get_error() {
    lucy_Err *error
        = (lucy_Err*)lucy_Host_callback_obj(LUCY_ERR, "get_error", 0);
    CFISH_DECREF(error); // Cancel out incref from callback.
    return error;
}

void
lucy_Err_set_error(lucy_Err *error) {
    lucy_Host_callback(LUCY_ERR, "set_error", 1,
                       CFISH_ARG_OBJ("error", error));
    CFISH_DECREF(error);
}

void
lucy_Err_do_throw(lucy_Err *err) {
    dSP;
    SV *error_sv = (SV*)Lucy_Err_To_Host(err);
    CFISH_DECREF(err);
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(sv_2mortal(error_sv));
    PUTBACK;
    call_pv("Clownfish::Err::do_throw", G_DISCARD);
    FREETMPS;
    LEAVE;
}

void*
lucy_Err_to_host(lucy_Err *self) {
    Lucy_Err_To_Host_t super_to_host
        = CFISH_SUPER_METHOD_PTR(LUCY_ERR, Lucy_Err_To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

void
lucy_Err_throw_mess(lucy_VTable *vtable, lucy_CharBuf *message) {
    Lucy_Err_Make_t make
        = CFISH_METHOD_PTR(CFISH_CERTIFY(vtable, LUCY_VTABLE), Lucy_Err_Make);
    lucy_Err *err = (lucy_Err*)CFISH_CERTIFY(make(NULL), LUCY_ERR);
    Lucy_Err_Cat_Mess(err, message);
    CFISH_DECREF(message);
    lucy_Err_do_throw(err);
}

void
lucy_Err_warn_mess(lucy_CharBuf *message) {
    SV *error_sv = XSBind_cb_to_sv(message);
    CFISH_DECREF(message);
    warn("%s", SvPV_nolen(error_sv));
    SvREFCNT_dec(error_sv);
}

lucy_Err*
lucy_Err_trap(Cfish_Err_Attempt_t routine, void *context) {
    lucy_Err *error = NULL;
    SV *routine_sv = newSViv(PTR2IV(routine));
    SV *context_sv = newSViv(PTR2IV(context));
    dSP;
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    EXTEND(SP, 2);
    PUSHs(sv_2mortal(routine_sv));
    PUSHs(sv_2mortal(context_sv));
    PUTBACK;

    int count = call_sv(attempt_xsub, G_EVAL | G_DISCARD);
    if (count != 0) {
        lucy_CharBuf *mess
            = lucy_CB_newf("'attempt' returned too many values: %i32",
                           (int32_t)count);
        error = lucy_Err_new(mess);
    }
    else {
        SV *dollar_at = get_sv("@", FALSE);
        if (SvTRUE(dollar_at)) {
            if (sv_isobject(dollar_at)
                && sv_derived_from(dollar_at,"Clownfish::Err")
               ) {
                IV error_iv = SvIV(SvRV(dollar_at));
                error = INT2PTR(lucy_Err*, error_iv);
                CFISH_INCREF(error);
            }
            else {
                STRLEN len;
                char *ptr = SvPVutf8(dollar_at, len);
                lucy_CharBuf *mess = lucy_CB_new_from_trusted_utf8(ptr, len);
                error = lucy_Err_new(mess);
            }
        }
    }
    FREETMPS;
    LEAVE;

    return error;
}

