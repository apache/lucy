package KinoSearch::Store::RAMFolder;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $folder = KinoSearch::Store::RAMFolder->new;
    
    # or sometimes...
    my $folder = KinoSearch::Store::RAMFolder->new(
        path => $relative_path,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $folder = KinoSearch::Store::RAMFolder->new(
        path => $relative_path,   # default: empty string
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Store::RAMFolder",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

