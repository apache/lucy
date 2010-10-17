package KinoSearch::Search::RangeQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    # Match all articles by "Foo" published since the year 2000.
    my $range_query = KinoSearch::Search::RangeQuery->new(
        field         => 'publication_date',
        lower_term    => '2000-01-01',
        include_lower => 1,
    );
    my $author_query = KinoSearch::Search::TermQuery->new(
        field => 'author_last_name',
        text  => 'Foo',
    );
    my $and_query = KinoSearch::Search::ANDQuery->new(
        children => [ $range_query, $author_query ],
    );
    my $hits = $searcher->hits( query => $and_query );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $range_query = KinoSearch::Search::RangeQuery->new(
        field         => 'product_number', # required
        lower_term    => '003',            # see below
        upper_term    => '060',            # see below
        include_lower => 0,                # default true
        include_upper => 0,                # default true
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::RangeQuery",
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


