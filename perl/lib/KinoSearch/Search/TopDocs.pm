package KinoSearch::Search::TopDocs;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::TopDocs",
    bind_methods => [
        qw(
            Get_Match_Docs
            Get_Total_Hits
            Set_Total_Hits
            )
    ],
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

