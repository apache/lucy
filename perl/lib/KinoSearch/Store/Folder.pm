package KinoSearch::Store::Folder;
use KinoSearch;

1;

__END__

__BINDING__

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Store::Folder",
    bind_methods => [
        qw(
            Open_Out
            Open_In
            MkDir
            List_R
            Exists
            Rename
            Hard_Link
            Delete
            Slurp_File
            Close
            Get_Path
            )
    ],
    bind_constructors => ["new"],
    make_pod          => { synopsis => "    # Abstract base class.\n", },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

