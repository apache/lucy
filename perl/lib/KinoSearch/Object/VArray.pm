package KinoSearch::Object::VArray;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Object::VArray

SV*
shallow_copy(self)
    kino_VArray *self;
CODE:
    RETVAL = KINO_OBJ_TO_SV_NOINC(Kino_VA_Shallow_Copy(self));
OUTPUT: RETVAL

SV*
_deserialize(either_sv, instream)
    SV *either_sv;
    kino_InStream *instream;
CODE:
    CHY_UNUSED_VAR(either_sv);
    RETVAL = KINO_OBJ_TO_SV_NOINC(kino_VA_deserialize(NULL, instream));
OUTPUT: RETVAL

SV*
_clone(self)
    kino_VArray *self;
CODE:
    RETVAL = KINO_OBJ_TO_SV_NOINC(Kino_VA_Clone(self));
OUTPUT: RETVAL

SV*
shift(self)
    kino_VArray *self;
CODE:
    RETVAL = KINO_OBJ_TO_SV_NOINC(Kino_VA_Shift(self));
OUTPUT: RETVAL

SV*
pop(self)
    kino_VArray *self;
CODE:
    RETVAL = KINO_OBJ_TO_SV_NOINC(Kino_VA_Pop(self));
OUTPUT: RETVAL

SV*
delete(self, tick)
    kino_VArray *self;
    uint32_t    tick;
CODE:
    RETVAL = KINO_OBJ_TO_SV_NOINC(Kino_VA_Delete(self, tick));
OUTPUT: RETVAL

void
store(self, tick, value);
    kino_VArray *self; 
    uint32_t     tick;
    kino_Obj    *value;
PPCODE:
{
    if (value) { KINO_INCREF(value); }
    kino_VA_store(self, tick, value);
}

SV*
fetch(self, tick)
    kino_VArray *self;
    uint32_t     tick;
CODE:
    RETVAL = KINO_OBJ_TO_SV(Kino_VA_Fetch(self, tick));
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Object::VArray",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Push
            Push_VArray
            Unshift
            Excise
            Resize
            Get_Size
            )
    ],
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

