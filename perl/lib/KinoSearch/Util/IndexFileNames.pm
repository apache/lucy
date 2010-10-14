package KinoSearch::Util::IndexFileNames;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS_CODE';
MODULE = KinoSearch   PACKAGE = KinoSearch::Util::IndexFileNames

uint64_t
extract_gen(name)
    const kino_CharBuf *name;
CODE:
    RETVAL = kino_IxFileNames_extract_gen(name);
OUTPUT: RETVAL

SV*
latest_snapshot(folder)
    kino_Folder *folder;
CODE:
{
    kino_CharBuf *latest = kino_IxFileNames_latest_snapshot(folder);
    RETVAL = XSBind_cb_to_sv(latest);   
    KINO_DECREF(latest);
}
OUTPUT: RETVAL
END_XS_CODE

Clownfish::Binding::Perl::Class->register(
    parcel     => "KinoSearch",
    class_name => "KinoSearch::Util::IndexFileNames",
    xs_code    => $xs_code,
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

