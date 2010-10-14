package KinoSearch::Object::Host;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Object::Host

=for comment

These are all for testing purposes only.

=cut

IV
_test(...)
CODE:
    RETVAL = items;
OUTPUT: RETVAL

SV*
_test_obj(...)
CODE:
{
    kino_ByteBuf *test_obj = kino_BB_new_bytes("blah", 4);
    SV *pack_var = get_sv("KinoSearch::Object::Host::testobj", 1);
    RETVAL = (SV*)Kino_BB_To_Host(test_obj);
    SvSetSV_nosteal(pack_var, RETVAL);
    KINO_DECREF(test_obj);
    CHY_UNUSED_VAR(items);
}
OUTPUT: RETVAL

void
_callback(obj)
    kino_Obj *obj;
PPCODE:
{
    kino_ZombieCharBuf *blank = KINO_ZCB_BLANK(); 
    kino_Host_callback(obj, "_test", 2, 
        KINO_ARG_OBJ("nothing", (kino_CharBuf*)blank),
        KINO_ARG_I32("foo", 3));
}

int64_t
_callback_i64(obj)
    kino_Obj *obj;
CODE:
{
    kino_ZombieCharBuf *blank = KINO_ZCB_BLANK();
    RETVAL = kino_Host_callback_i64(obj, "_test", 2, 
        KINO_ARG_OBJ("nothing", (kino_CharBuf*)blank), 
        KINO_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

double
_callback_f64(obj)
    kino_Obj *obj;
CODE:
{
    kino_ZombieCharBuf *blank = KINO_ZCB_BLANK();
    RETVAL = kino_Host_callback_f64(obj, "_test", 2, 
        KINO_ARG_OBJ("nothing", (kino_CharBuf*)blank), 
        KINO_ARG_I32("foo", 3));
}
OUTPUT: RETVAL

SV*
_callback_obj(obj)
    kino_Obj *obj;
CODE: 
{
    kino_Obj *other = kino_Host_callback_obj(obj, "_test_obj", 0);
    RETVAL = (SV*)Kino_Obj_To_Host(other);
    KINO_DECREF(other);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch::Object::Host",
    xs_code    => $xs_code,
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

