#include "xs/XSBind.h"

void
lucy_Err_do_throw(lucy_Err *err)
{
    dSP;
    SV *error_sv = (SV*)Lucy_Err_To_Host(err);
    LUCY_DECREF(err);
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs( sv_2mortal(error_sv) );
    PUTBACK;
    call_pv("Lucy::Object::Err::do_throw", G_DISCARD);
    FREETMPS;
    LEAVE;
}

void*
lucy_Err_to_host(lucy_Err *self)
{
    lucy_Err_to_host_t super_to_host 
        = (lucy_Err_to_host_t)LUCY_SUPER_METHOD(LUCY_ERR, Err, To_Host);
    SV *perl_obj = super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

void
lucy_Err_throw_mess(lucy_VTable *vtable, lucy_CharBuf *message) 
{
    lucy_Err_make_t make = (lucy_Err_make_t)LUCY_METHOD(
        LUCY_ASSERT_IS_A(vtable, LUCY_VTABLE), Err, Make);
    lucy_Err *err = (lucy_Err*)LUCY_ASSERT_IS_A(make(NULL), LUCY_ERR);
    Lucy_Err_Cat_Mess(err, message);
    LUCY_DECREF(message);
    lucy_Err_do_throw(err);
}

void
lucy_Err_warn_mess(lucy_CharBuf *message) 
{
    SV *error_sv = XSBind_cb_to_sv(message);
    LUCY_DECREF(message);
    warn(SvPV_nolen(error_sv));
    SvREFCNT_dec(error_sv);
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

