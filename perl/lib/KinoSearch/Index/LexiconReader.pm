package KinoSearch::Index::LexiconReader;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $lex_reader = $seg_reader->obtain("KinoSearch::Index::LexiconReader");
    my $lexicon    = $lex_reader->lexicon( field => 'title' );
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::LexiconReader",
    bind_methods      => [qw( Lexicon Doc_Freq Fetch_Term_Info )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( lexicon doc_freq )],
    },
);
Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::DefaultLexiconReader",
    bind_constructors => ["new"],
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

