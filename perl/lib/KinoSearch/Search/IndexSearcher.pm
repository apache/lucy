package KinoSearch::Search::IndexSearcher;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $searcher = KinoSearch::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
    my $hits = $searcher->hits(
        query      => 'foo bar',
        offset     => 0,
        num_wanted => 100,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $searcher = KinoSearch::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::IndexSearcher",
    bind_methods      => [qw( Get_Reader )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
        methods     => [
            qw( hits
                collect
                doc_max
                doc_freq
                fetch_doc
                get_schema
                get_reader )
        ],
    },
);


