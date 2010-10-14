package KinoSearch::Test::Util::BBSortEx;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch    PACKAGE = KinoSearch::Test::Util::BBSortEx

SV*
fetch(self)
    kino_BBSortEx *self;
CODE:
{
    void *address = Kino_BBSortEx_Fetch(self);
    if (address) {
        RETVAL = XSBind_kino_to_perl(*(kino_Obj**)address);
        KINO_DECREF(*(kino_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

SV*
peek(self)
    kino_BBSortEx *self;
CODE:
{
    void *address = Kino_BBSortEx_Peek(self);
    if (address) {
        RETVAL = XSBind_kino_to_perl(*(kino_Obj**)address);
    }
    else {
        RETVAL = newSV(0);
    }
}
OUTPUT: RETVAL

void
feed(self, bb)
    kino_BBSortEx *self;
    kino_ByteBuf *bb;
CODE:
    KINO_INCREF(bb);
    Kino_BBSortEx_Feed(self, &bb);

END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Test::Util::BBSortEx",
    bind_constructors => ["new"],
    xs_code           => $xs_code,
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

