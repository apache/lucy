package KinoSearch::Search::Span;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $combined_length = $upper_span->get_length
        + ( $upper_span->get_offset - $lower_span->get_offset );
    my $combined_span = KinoSearch::Search::Span->new(
        offset => $lower_span->get_offset,
        length => $combined_length,
    );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $span = KinoSearch::Search::Span->new(
        offset => 75,     # required
        length => 7,      # required
        weight => 1.0,    # default 0.0
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Span",
    bind_methods => [
        qw( Set_Offset
            Get_Offset
            Set_Length
            Get_Length
            Set_Weight
            Get_Weight )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw( set_offset
                get_offset
                set_length
                get_length
                set_weight
                get_weight )
        ],
    }
);

__COPYRIGHT__

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

