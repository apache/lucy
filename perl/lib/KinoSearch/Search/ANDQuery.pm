package KinoSearch::Search::ANDQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $foo_and_bar_query = KinoSearch::Search::ANDQuery->new(
        children => [ $foo_query, $bar_query ],
    );
    my $hits = $searcher->hits( query => $foo_and_bar_query );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $foo_and_bar_query = KinoSearch::Search::ANDQuery->new(
        children => [ $foo_query, $bar_query ],
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::ANDQuery",
    bind_constructors => ["new"],
    make_pod          => {
        methods     => [qw( add_child )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

