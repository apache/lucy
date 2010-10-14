package KinoSearch::Plan::Schema;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    use KinoSearch::Plan::Schema;
    use KinoSearch::Plan::FullTextType;
    use KinoSearch::Analysis::PolyAnalyzer;
    
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
    my $schema = MySchema->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Plan::Schema",
    bind_methods => [
        qw(
            Architecture
            Get_Architecture
            Get_Similarity
            Fetch_Type
            Fetch_Analyzer
            Fetch_Sim
            Num_Fields
            All_Fields
            Spec_Field
            Write
            Eat
            )
    ],
    bind_constructors => [qw( new )],
    make_pod          => {
        methods => [
            qw(
                spec_field
                num_fields
                all_fields
                fetch_type
                fetch_sim
                architecture
                get_architecture
                get_similarity
                )
        ],
        synopsis     => $synopsis,
        constructors => [ { sample => $constructor } ],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

