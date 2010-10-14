package KinoSearch::Object::ByteBuf;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Object::ByteBuf

SV*
new(either_sv, sv)
    SV *either_sv;
    SV *sv;
CODE:
{
    STRLEN size;
    char *ptr = SvPV(sv, size);
    kino_ByteBuf *self = (kino_ByteBuf*)XSBind_new_blank_obj(either_sv);
    kino_BB_init(self, size);
    Kino_BB_Mimic_Bytes(self, ptr, size);
    RETVAL = KINO_OBJ_TO_SV_NOINC(self);
}
OUTPUT: RETVAL

SV*
_deserialize(either_sv, instream)
    SV *either_sv;
    kino_InStream *instream;
CODE:
    CHY_UNUSED_VAR(either_sv);
    RETVAL = KINO_OBJ_TO_SV_NOINC(kino_BB_deserialize(NULL, instream));
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::ByteBuf",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Get_Size
            Get_Capacity
            Cat
            )
    ],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

