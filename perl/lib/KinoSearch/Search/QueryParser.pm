package KinoSearch::Search::QueryParser;
use KinoSearch;

1;

__END__

__BINDING__

my $synopsis = <<'END_SYNOPSIS';
    my $query_parser = KinoSearch::Search::QueryParser->new(
        schema => $searcher->get_schema,
        fields => ['body'],
    );
    my $query = $query_parser->parse( $query_string );
    my $hits  = $searcher->hits( query => $query );
END_SYNOPSIS

my $constructor = <<'END_CONSTRUCTOR';
    my $query_parser = KinoSearch::Search::QueryParser->new(
        schema         => $searcher->get_schema,    # required
        analyzer       => $analyzer,                # overrides schema
        fields         => ['bodytext'],             # default: indexed fields
        default_boolop => 'AND',                    # default: 'OR'
    );
END_CONSTRUCTOR

Clownfish::Binding::Perl::Class->register(
    parcel       => "KinoSearch",
    class_name   => "KinoSearch::Search::QueryParser",
    bind_methods => [
        qw(
            Parse
            Tree
            Expand
            Expand_Leaf
            Prune
            Heed_Colons
            Set_Heed_Colons
            Get_Analyzer
            Get_Schema
            Get_Fields
            Make_Term_Query
            Make_Phrase_Query
            Make_AND_Query
            Make_OR_Query
            Make_NOT_Query
            Make_Req_Opt_Query
            )
    ],
    bind_constructors => ["new"],
    make_pod          => {
        methods => [
            qw( parse
                tree
                expand
                expand_leaf
                prune
                set_heed_colons
                make_term_query
                make_phrase_query
                make_and_query
                make_or_query
                make_not_query
                make_req_opt_query
                )
        ],
        synopsis    => $synopsis,
        constructor => { sample => $constructor },
    }
);


