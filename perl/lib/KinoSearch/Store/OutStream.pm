package KinoSearch::Store::OutStream;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch     PACKAGE = KinoSearch::Store::OutStream

void
print(self, ...)
    kino_OutStream *self;
PPCODE:
{
    int i;
    for (i = 1; i < items; i++) {
        STRLEN len;
        char *ptr = SvPV( ST(i), len);
        Kino_OutStream_Write_Bytes(self, ptr, len);
    }
}

void 
write_string(self, aSV)
    kino_OutStream *self;
    SV *aSV;
PPCODE:
{
    STRLEN len = 0;
    char *ptr = SvPVutf8(aSV, len);
    Kino_OutStream_Write_C32(self, len);
    Kino_OutStream_Write_Bytes(self, ptr, len);
}
END_XS_CODE

my $synopsis = <<'END_SYNOPSIS';    # Don't use this yet.
    my $outstream = $folder->open_out($filename) or die $@;
    $outstream->write_u64($file_position);
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Store::OutStream",
    xs_code      => $xs_code,
    bind_methods => [
        qw(
            Tell
            Length
            Flush
            Close
            Absorb
            Write_I8
            Write_I32
            Write_I64
            Write_U8
            Write_U32
            Write_U64
            Write_C32
            Write_C64
            Write_F32
            Write_F64
            )
    ],
    bind_constructors => ['open|do_open'],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

