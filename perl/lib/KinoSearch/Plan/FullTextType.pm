package KinoSearch::Plan::FullTextType;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        language => 'en',
    );
    my $type = KinoSearch::Plan::FullTextType->new(
        analyzer => $polyanalyzer,
    );
    my $schema = KinoSearch::Plan::Schema->new;
    $schema->spec_field( name => 'title',   type => $type );
    $schema->spec_field( name => 'content', type => $type );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $type = KinoSearch::Plan::FullTextType->new(
        analyzer      => $analyzer,    # required
        boost         => 2.0,          # default: 1.0
        indexed       => 1,            # default: true
        stored        => 1,            # default: true
        sortable      => 1,            # default: false
        highlightable => 1,            # default: false
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Plan::FullTextType",
    bind_constructors => ["new|init2"],
    bind_methods      => [
        qw(
            Set_Highlightable
            Highlightable
            )
    ],
    make_pod => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw(
                set_highlightable
                highlightable
                )
        ],
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

