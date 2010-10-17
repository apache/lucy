package KinoSearch::Index::Lexicon;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $lexicon = $index_reader->lexicon( field => 'content' );
    while ( $lexicon->next ) {
       print $lexicon->get_term . "\n";
    }
END_SYNOPSIS

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Index::Lexicon",
    bind_methods      => [qw( Seek Next Reset Get_Term Get_Field )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis => $synopsis,
        methods  => [qw( seek next get_term reset )],
    },
);


