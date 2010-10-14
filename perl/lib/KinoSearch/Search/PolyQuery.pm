package KinoSearch::Search::PolyQuery;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::PolyQuery",
    bind_methods      => [qw( Add_Child Set_Children Get_Children )],
    bind_constructors => ["new"],
    make_pod          => {}
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

