package KinoSearch::Search::SortSpec;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = KinoSearch::Search::SortSpec->new(
        rules => [
            KinoSearch::Search::SortRule->new( field => 'date' ),
            KinoSearch::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
    my $hits = $searcher->hits(
        query     => $query,
        sort_spec => $sort_spec,
    );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $sort_spec = KinoSearch::Search::SortSpec->new( rules => \@rules );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel            => "KinoSearch",
    class_name        => "KinoSearch::Search::SortSpec",
    bind_methods      => [qw( Get_Rules )],
    bind_constructors => ["new"],
    make_pod          => {
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


