package KinoSearch::Search::Searcher;
use KinoSearch;

1;

__END__

__BINDING__

my $constructor = <<'END_CONSTRUCTOR';
    package MySearcher;
    use base qw( KinoSearch::Search::Searcher );
    sub new {
        my $self = shift->SUPER::new;
        ...
        return $self;
    }
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::Searcher",
    bind_methods => [
        qw( Doc_Max
            Doc_Freq
            Glean_Query
            Hits
            Collect
            Top_Docs
            Fetch_Doc
            Fetch_Doc_Vec
            Get_Schema
            Close )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => "    # Abstract base class.\n",
        constructor => { sample => $constructor },
        methods     => [
            qw(
                hits
                collect
                glean_query
                doc_max
                doc_freq
                fetch_doc
                get_schema
                )
        ],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

