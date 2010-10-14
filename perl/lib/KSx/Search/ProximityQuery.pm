package KSx::Search::ProximityQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $proximity_query = KSx::Search::ProximityQuery->new( 
        field  => 'content',
        terms  => [qw( the who )],
        within => 10,    # match within 10 positions
    );
    my $hits = $searcher->hits( query => $proximity_query );
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KSx::Search::ProximityQuery",
    bind_methods      => [qw( Get_Field Get_Terms )],
    bind_constructors => ["new"],
    make_pod          => {
        constructor => { sample => '' },
        synopsis    => $synopsis,
        methods     => [qw( get_field get_terms get_within )],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KSx::Search::ProximityCompiler",
    bind_constructors => ["do_new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

