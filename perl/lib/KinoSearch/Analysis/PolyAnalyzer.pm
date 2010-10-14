package KinoSearch::Analysis::PolyAnalyzer;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $schema = KinoSearch::Plan::Schema->new;
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new( 
        language => 'en',
    );
    my $type = KinoSearch::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $analyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        language  => 'es',
    );
    
    # or...

    my $case_folder  = KinoSearch::Analysis::CaseFolder->new;
    my $tokenizer    = KinoSearch::Analysis::Tokenizer->new;
    my $stemmer      = KinoSearch::Analysis::Stemmer->new( language => 'en' );
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $whitespace_tokenizer, $stemmer, ], );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::PolyAnalyzer",
    bind_constructors => ["new"],
    bind_methods      => [qw( Get_Analyzers )],
    make_pod          => {
        methods     => [qw( get_analyzers )],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.
