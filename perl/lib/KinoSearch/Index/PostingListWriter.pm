package KinoSearch::Index::PostingListWriter;
use KinoSearch;

1;

__END__

__BINDING__

my $xs_code = <<'END_XS';
MODULE = KinoSearch    PACKAGE = KinoSearch::Index::PostingListWriter

void
set_default_mem_thresh(mem_thresh)
    size_t mem_thresh;
PPCODE:
    kino_PListWriter_set_default_mem_thresh(mem_thresh);
END_XS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::PostingListWriter",
    xs_code           => $xs_code,
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

