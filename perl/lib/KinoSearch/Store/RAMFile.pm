package KinoSearch::Store::RAMFile;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::RAMFile",
    bind_methods      => [qw( Get_Contents )],
    bind_constructors => ['new'],
);

__COPYRIGHT__

Copyright 2009-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

