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
#include "Lucy/Object/Host.h"

lucy_Err*
lucy_Err_get_error() {
    lucy_Err *error
        = (lucy_Err*)lucy_Host_callback_obj(LUCY_ERR, "get_error", 0);
    LUCY_DECREF(error); // Cancel out incref from callback.
    return error;
}

void
lucy_Err_set_error(lucy_Err *error) {
    lucy_Host_callback(LUCY_ERR, "set_error", 1,
                       CFISH_ARG_OBJ("error", error));
    LUCY_DECREF(error);
}

void
lucy_Err_do_throw(lucy_Err *err) {
    dSP;
    SV *error_sv = (SV*)Lucy_Err_To_Host(err);
    LUCY_DECREF(err);
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs(sv_2mortal(error_sv));
    PUTBACK;
    call_pv("Lucy::Object::Err::do_throw", G_DISCARD);
    FREETMPS;
    LEAVE;
}

void*
lucy_Err_to_host(lucy_Err *self) {
    lucy_Err_to_host_t super_to_host
        = (lucy_Err_to_host_t)LUCY_SUPER_METHOD(LUCY_ERR, Err, To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

void
lucy_Err_throw_mess(lucy_VTable *vtable, lucy_CharBuf *message) {
    lucy_Err_make_t make = (lucy_Err_make_t)LUCY_METHOD(
                               CFISH_CERTIFY(vtable, LUCY_VTABLE), Err, Make);
    lucy_Err *err = (lucy_Err*)CFISH_CERTIFY(make(NULL), LUCY_ERR);
    Lucy_Err_Cat_Mess(err, message);
    LUCY_DECREF(message);
    lucy_Err_do_throw(err);
}

void
lucy_Err_warn_mess(lucy_CharBuf *message) {
    SV *error_sv = XSBind_cb_to_sv(message);
    LUCY_DECREF(message);
    warn("%s", SvPV_nolen(error_sv));
    SvREFCNT_dec(error_sv);
}


