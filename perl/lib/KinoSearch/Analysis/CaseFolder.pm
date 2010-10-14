package KinoSearch::Analysis::CaseFolder;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $case_folder = KinoSearch::Analysis::CaseFolder->new;

    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $tokenizer, $stemmer ],
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $case_folder = KinoSearch::Analysis::CaseFolder->new;
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::CaseFolder",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);

__COPYRIGHT__

Copyright 2005-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify
under the same terms as Perl itself.

