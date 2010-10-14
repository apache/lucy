package KinoSearch::Search::Hits;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $hits = $searcher->hits(
        query      => $query,
        offset     => 0,
        num_wanted => 10,
    );
    while ( my $hit = $hits->next ) {
        print "<p>$hit->{title} <em>" . $hit->get_score . "</em></p>\n";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Hits",
    bind_methods => [
        qw(
            Total_Hits
            Next
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( next total_hits )],
    }
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

