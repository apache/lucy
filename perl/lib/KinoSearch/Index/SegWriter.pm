package KinoSearch::Index::SegWriter;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::SegWriter",
    bind_constructors => ["new"],
    bind_methods      => [
        qw(
            Add_Writer
            Register
            Fetch
            )
    ],
    make_pod => {
        methods => [
            qw(
                add_doc
                add_writer
                register
                fetch
                )
        ],
    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

