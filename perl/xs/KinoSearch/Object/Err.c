#include "xs/XSBind.h"
#include "KinoSearch/Object/Host.h"

kino_Err*
kino_Err_get_error()
{
    kino_Err *error 
        = (kino_Err*)kino_Host_callback_obj(KINO_ERR, "get_error", 0);
    KINO_DECREF(error); // Cancel out incref from callback. 
    return error;
}

void
kino_Err_set_error(kino_Err *error)
{
    kino_Host_callback(KINO_ERR, "set_error", 1, 
        KINO_ARG_OBJ("error", error));
    KINO_DECREF(error);
}

void
kino_Err_do_throw(kino_Err *err)
{
    dSP;
    SV *error_sv = (SV*)Kino_Err_To_Host(err);
    KINO_DECREF(err);
    ENTER;
    SAVETMPS;
    PUSHMARK(SP);
    XPUSHs( sv_2mortal(error_sv) );
    PUTBACK;
    call_pv("KinoSearch::Object::Err::do_throw", G_DISCARD);
    FREETMPS;
    LEAVE;
}

void*
kino_Err_to_host(kino_Err *self)
{
    kino_Err_to_host_t super_to_host 
        = (kino_Err_to_host_t)KINO_SUPER_METHOD(KINO_ERR, Err, To_Host);
    SV *perl_obj = (SV*)super_to_host(self);
    XSBind_enable_overload(perl_obj);
    return perl_obj;
}

void
kino_Err_throw_mess(kino_VTable *vtable, kino_CharBuf *message) 
{
    kino_Err_make_t make = (kino_Err_make_t)KINO_METHOD(
        KINO_CERTIFY(vtable, KINO_VTABLE), Err, Make);
    kino_Err *err = (kino_Err*)KINO_CERTIFY(make(NULL), KINO_ERR);
    Kino_Err_Cat_Mess(err, message);
    KINO_DECREF(message);
    kino_Err_do_throw(err);
}

void
kino_Err_warn_mess(kino_CharBuf *message) 
{
    SV *error_sv = XSBind_cb_to_sv(message);
    KINO_DECREF(message);
    warn(SvPV_nolen(error_sv));
    SvREFCNT_dec(error_sv);
}

/* Copyright 2006-2010 Marvin Humphrey
 *
 * This program is free software; you can redistribute it and/or modify
 * under the same terms as Perl itself.
 */

