package KinoSearch::Analysis::Stopalizer;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $stopalizer = KinoSearch::Analysis::Stopalizer->new(
        language => 'fr',
    );
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $tokenizer, $stopalizer, $stemmer ],
    );

This class uses Lingua::StopWords for its default stoplists, so it supports
the same set of languages.
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $stopalizer = KinoSearch::Analysis::Stopalizer->new(
        language => 'de',
    );
    
    # or...
    my $stopalizer = KinoSearch::Analysis::Stopalizer->new(
        stoplist => \%stoplist,
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Stopalizer",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor }
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself

