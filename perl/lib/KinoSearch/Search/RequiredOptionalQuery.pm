package KinoSearch::Search::RequiredOptionalQuery;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $foo_and_maybe_bar = KinoSearch::Search::RequiredOptionalQuery->new(
        required_query => $foo_query,
        optional_query => $bar_query,
    );
    my $hits = $searcher->hits( query => $foo_and_maybe_bar );
    ...
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $reqopt_query = KinoSearch::Search::RequiredOptionalQuery->new(
        required_query => $foo_query,    # required
        optional_query => $bar_query,    # required
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::RequiredOptionalQuery",
    bind_methods => [
        qw( Get_Required_Query Set_Required_Query
            Get_Optional_Query Set_Optional_Query )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        methods => [
            qw( get_required_query set_required_query
                get_optional_query set_optional_query )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    },
);


