package KinoSearch::Plan::BlobType;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $string_type = KinoSearch::Plan::StringType->new;
    my $blob_type   = KinoSearch::Plan::BlobType->new( stored => 1 );
    my $schema      = KinoSearch::Plan::Schema->new;
    $schema->spec_field( name => 'id',   type => $string_type );
    $schema->spec_field( name => 'jpeg', type => $blob_type );
END_SYNOPSIS
my $constructor = <<'END_CONSTRUCTOR';
    my $blob_type = KinoSearch::Plan::BlobType->new(
        stored => 1,  # default: false
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Plan::BlobType",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

