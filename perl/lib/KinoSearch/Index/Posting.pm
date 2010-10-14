package KinoSearch::Index::Posting;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Index::Posting",
    bind_methods => [qw( Get_Doc_ID )],
#    make_pod => {
#        synopsis => "    # Abstract base class.\n",
#    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

