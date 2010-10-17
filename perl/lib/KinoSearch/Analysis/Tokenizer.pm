package KinoSearch::Analysis::Tokenizer;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $whitespace_tokenizer
        = KinoSearch::Analysis::Tokenizer->new( pattern => '\S+' );

    # or...
    my $word_char_tokenizer
        = KinoSearch::Analysis::Tokenizer->new( pattern => '\w+' );

    # or...
    my $apostrophising_tokenizer = KinoSearch::Analysis::Tokenizer->new;

    # Then... once you have a tokenizer, put it into a PolyAnalyzer:
    my $polyanalyzer = KinoSearch::Analysis::PolyAnalyzer->new(
        analyzers => [ $case_folder, $word_char_tokenizer, $stemmer ], );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $word_char_tokenizer = KinoSearch::Analysis::Tokenizer->new(
        pattern => '\w+',    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Analysis::Tokenizer",
    bind_methods      => [qw( Set_Token_RE )],
    bind_constructors => ["_new"],
    make_pod          => {
        constructor => { sample => $constructor },
        synopsis    => $synopsis,
    },
);


