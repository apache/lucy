# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
package Lucy::Build::Binding::Search;

sub bind_all {
    my $class = shift;
    $class->bind_andmatcher;
    $class->bind_andquery;
    $class->bind_bitvecmatcher;
    $class->bind_collector;
    $class->bind_compiler;
    $class->bind_hitqueue;
    $class->bind_hits;
    $class->bind_indexsearcher;
    $class->bind_leafquery;
    $class->bind_matchallquery;
    $class->bind_matchdoc;
    $class->bind_matcher;
    $class->bind_notmatcher;
    $class->bind_notquery;
    $class->bind_nomatchquery;
    $class->bind_orquery;
    $class->bind_orscorer;
    $class->bind_phrasequery;
    $class->bind_polyquery;
    $class->bind_polysearcher;
    $class->bind_query;
    $class->bind_queryparser;
    $class->bind_rangequery;
    $class->bind_requiredoptionalmatcher;
    $class->bind_requiredoptionalquery;
    $class->bind_searcher;
    $class->bind_sortrule;
    $class->bind_sortspec;
    $class->bind_span;
    $class->bind_termquery;
    $class->bind_topdocs;
}

sub bind_andmatcher {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ANDMatcher",
        bind_constructors => ["new"],
    );

}

sub bind_andquery {
    my $synopsis = <<'END_SYNOPSIS';
    my $foo_and_bar_query = Lucy::Search::ANDQuery->new(
        children => [ $foo_query, $bar_query ],
    );
    my $hits = $searcher->hits( query => $foo_and_bar_query );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $foo_and_bar_query = Lucy::Search::ANDQuery->new(
        children => [ $foo_query, $bar_query ],
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ANDQuery",
        bind_constructors => ["new"],
        make_pod          => {
            methods     => [qw( add_child )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );

}

sub bind_bitvecmatcher {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::BitVecMatcher",
        bind_constructors => [qw( new )],
    );

}

sub bind_collector {
    my $constructor = <<'END_CONSTRUCTOR';
    package MyCollector;
    use base qw( Lucy::Search::Collector );
    our %foo;
    sub new {
        my $self = shift->SUPER::new;
        my %args = @_;
        $foo{$$self} = $args{foo};
        return $self;
    }
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Collector",
        bind_methods => [
            qw(
                Collect
                Set_Reader
                Set_Base
                Set_Matcher
                Need_Score
                )
        ],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => "    # Abstract base class.\n",
            constructor => { sample => $constructor },
            methods     => [qw( collect )],
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::Collector::OffsetCollector",
        bind_constructors => ["new"],
    );

}

sub bind_compiler {
    my $synopsis = <<'END_SYNOPSIS';
    # (Compiler is an abstract base class.)
    package MyCompiler;
    use base qw( Lucy::Search::Compiler );

    sub make_matcher {
        my $self = shift;
        return MyMatcher->new( @_, compiler => $self );
    }
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $compiler = MyCompiler->SUPER::new(
        parent     => $my_query,
        searcher   => $searcher,
        similarity => $sim,        # default: undef
        boost      => undef,       # default: see below
    );
END_CONSTRUCTOR_CODE_SAMPLE

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Compiler",
        bind_methods => [
            qw(
                Make_Matcher
                Get_Parent
                Get_Similarity
                Get_Weight
                Sum_Of_Squared_Weights
                Apply_Norm_Factor
                Normalize
                Highlight_Spans
                )
        ],
        bind_constructors => ["do_new"],
        make_pod          => {
            methods => [
                qw(
                    make_matcher
                    get_weight
                    sum_of_squared_weights
                    apply_norm_factor
                    normalize
                    get_parent
                    get_similarity
                    highlight_spans
                    )
            ],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        }
    );

}

sub bind_hitqueue {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::HitQueue",
        bind_constructors => ["new"],
    );

}

sub bind_hits {
    my $synopsis = <<'END_SYNOPSIS';
    my $hits = $searcher->hits(
        query      => $query,
        offset     => 0,
        num_wanted => 10,
    );
    while ( my $hit = $hits->next ) {
        print "<p>$hit->{title} <em>" . $hit->get_score . "</em></p>\n";
    }
END_SYNOPSIS

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Hits",
        bind_methods => [
            qw(
                Total_Hits
                Next
                )
        ],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( next total_hits )],
        }
    );

}

sub bind_indexsearcher {
    my $synopsis = <<'END_SYNOPSIS';
    my $searcher = Lucy::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
    my $hits = $searcher->hits(
        query      => 'foo bar',
        offset     => 0,
        num_wanted => 100,
    );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $searcher = Lucy::Search::IndexSearcher->new( 
        index => '/path/to/index' 
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::IndexSearcher",
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

}

sub bind_leafquery {
    my $synopsis = <<'END_SYNOPSIS';
    package MyQueryParser;
    use base qw( Lucy::Search::QueryParser );

    sub expand_leaf {
        my ( $self, $leaf_query ) = @_;
        if ( $leaf_query->get_text =~ /.\*\s*$/ ) {
            return PrefixQuery->new(
                query_string => $leaf_query->get_text,
                field        => $leaf_query->get_field,
            );
        }
        else {
            return $self->SUPER::expand_leaf($leaf_query);
        }
    }
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $leaf_query = Lucy::Search::LeafQuery->new(
        text  => '"three blind mice"',    # required
        field => 'content',               # default: undef
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::LeafQuery",
        bind_methods      => [qw( Get_Field Get_Text )],
        bind_constructors => ["new"],
        make_pod          => {
            methods     => [qw( get_field get_text )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        }
    );

}

sub bind_matchallquery {
    my $constructor = <<'END_CONSTRUCTOR';
    my $match_all_query = Lucy::Search::MatchAllQuery->new;
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::MatchAllQuery",
        bind_constructors => ["new"],
        make_pod          => { constructor => { sample => $constructor }, }
    );

}

sub bind_matchdoc {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::MatchDoc",
        bind_methods => [
            qw(
                Get_Doc_ID
                Set_Doc_ID
                Get_Score
                Set_Score
                Get_Values
                Set_Values
                )
        ],
        bind_constructors => ["new"],
    );

}

sub bind_matcher {
    my $synopsis = <<'END_SYNOPSIS';
    # abstract base class
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $matcher = MyMatcher->SUPER::new;
END_CONSTRUCTOR_CODE_SAMPLE

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::Matcher",
        bind_methods      => [qw( Next Advance Get_Doc_ID Score Collect )],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( next advance get_doc_id score )],
        }
    );

}

sub bind_notmatcher {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NOTMatcher",
        bind_constructors => ["new"],
    );

}

sub bind_notquery {
    my $synopsis = <<'END_SYNOPSIS';
    my $not_bar_query = Lucy::Search::NOTQuery->new( 
        negated_query => $bar_query,
    );
    my $foo_and_not_bar_query = Lucy::Search::ANDQuery->new(
        children => [ $foo_query, $not_bar_query ].
    );
    my $hits = $searcher->hits( query => $foo_and_not_bar_query );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $not_query = Lucy::Search::NOTQuery->new( 
        negated_query => $query,
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NOTQuery",
        bind_constructors => ["new"],
        bind_methods      => [qw( Get_Negated_Query Set_Negated_Query )],
        make_pod          => {
            methods     => [qw( get_negated_query set_negated_query )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        }
    );

}

sub bind_nomatchquery {
    my $constructor = <<'END_CONSTRUCTOR';
    my $no_match_query = Lucy::Search::NoMatchQuery->new;
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NoMatchQuery",
        bind_constructors => ["new"],
        make_pod          => { constructor => { sample => $constructor }, }
    );

}

sub bind_orquery {
    my $synopsis = <<'END_SYNOPSIS';
    my $foo_or_bar_query = Lucy::Search::ORQuery->new(
        children => [ $foo_query, $bar_query ],
    );
    my $hits = $searcher->hits( query => $foo_or_bar_query );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $foo_or_bar_query = Lucy::Search::ORQuery->new(
        children => [ $foo_query, $bar_query ],
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ORQuery",
        bind_constructors => ["new"],
        make_pod          => {
            methods     => [qw( add_child )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor, }
        },
    );

}

sub bind_orscorer {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ORScorer",
        bind_constructors => ["new"],
    );

}

sub bind_phrasequery {
    my $synopsis = <<'END_SYNOPSIS';
    my $phrase_query = Lucy::Search::PhraseQuery->new( 
        field => 'content',
        terms => [qw( the who )],
    );
    my $hits = $searcher->hits( query => $phrase_query );
END_SYNOPSIS

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PhraseQuery",
        bind_methods      => [qw( Get_Field Get_Terms )],
        bind_constructors => ["new"],
        make_pod          => {
            constructor => { sample => '' },
            synopsis    => $synopsis,
            methods     => [qw( get_field get_terms )],
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PhraseCompiler",
        bind_constructors => ["do_new"],
    );

}

sub bind_polyquery {
    my $synopsis = <<'END_SYNOPSIS';
    sub walk {
        my $query = shift;
        if ( $query->isa("Lucy::Search::PolyQuery") ) {
            if    ( $query->isa("Lucy::Search::ORQuery") )  { ... }
            elsif ( $query->isa("Lucy::Search::ANDQuery") ) { ... }
            elsif ( $query->isa("Lucy::Search::RequiredOptionalQuery") ) {
                ...
            }
            elsif ( $query->isa("Lucy::Search::NOTQuery") ) { ... }
        }
        else { ... }
    }
END_SYNOPSIS

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PolyQuery",
        bind_methods      => [qw( Add_Child Set_Children Get_Children )],
        bind_constructors => ["new"],
        make_pod          => { synopsis => $synopsis, },
    );

}

sub bind_polysearcher {
    my $synopsis = <<'END_SYNOPSIS';
    my $schema = MySchema->new;
    for my $index (@index_paths) {
        push @searchers, Lucy::Search::IndexSearcher->new( index => $index );
    }
    my $poly_searcher = Lucy::Search::PolySearcher->new(
        schema    => $schema,
        searchers => \@searchers,
    );
    my $hits = $poly_searcher->hits( query => $query );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $poly_searcher = Lucy::Search::PolySearcher->new(
        schema    => $schema,
        searchers => \@searchers,
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PolySearcher",
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [
                qw( hits
                    doc_max
                    doc_freq
                    fetch_doc
                    get_schema
                    )
            ],
        }
    );

}

sub bind_query {
    my $synopsis = <<'END_SYNOPSIS';
    # Query is an abstract base class.
    package MyQuery;
    use base qw( Lucy::Search::Query );
    
    sub make_compiler {
        my ( $self, %args ) = @_;
        my $subordinate = delete $args{subordinate};
        my $compiler = MyCompiler->new( %args, parent => $self );
        $compiler->normalize unless $subordinate;
        return $compiler;
    }
    
    package MyCompiler;
    use base ( Lucy::Search::Compiler );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $query = MyQuery->SUPER::new(
        boost => 2.5,
    );
END_CONSTRUCTOR_CODE_SAMPLE

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Query",
        bind_methods => [
            qw( Set_Boost
                Get_Boost
                _make_compiler|Make_Compiler )
        ],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( make_compiler set_boost get_boost )],
        },
    );

}

sub bind_queryparser {
    my $synopsis = <<'END_SYNOPSIS';
    my $query_parser = Lucy::Search::QueryParser->new(
        schema => $searcher->get_schema,
        fields => ['body'],
    );
    my $query = $query_parser->parse( $query_string );
    my $hits  = $searcher->hits( query => $query );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $query_parser = Lucy::Search::QueryParser->new(
        schema         => $searcher->get_schema,    # required
        analyzer       => $analyzer,                # overrides schema
        fields         => ['bodytext'],             # default: indexed fields
        default_boolop => 'AND',                    # default: 'OR'
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::QueryParser",
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

}

sub bind_rangequery {
    my $synopsis = <<'END_SYNOPSIS';
    # Match all articles by "Foo" published since the year 2000.
    my $range_query = Lucy::Search::RangeQuery->new(
        field         => 'publication_date',
        lower_term    => '2000-01-01',
        include_lower => 1,
    );
    my $author_query = Lucy::Search::TermQuery->new(
        field => 'author_last_name',
        text  => 'Foo',
    );
    my $and_query = Lucy::Search::ANDQuery->new(
        children => [ $range_query, $author_query ],
    );
    my $hits = $searcher->hits( query => $and_query );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $range_query = Lucy::Search::RangeQuery->new(
        field         => 'product_number', # required
        lower_term    => '003',            # see below
        upper_term    => '060',            # see below
        include_lower => 0,                # default true
        include_upper => 0,                # default true
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::RangeQuery",
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );

}

sub bind_requiredoptionalmatcher {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::RequiredOptionalMatcher",
        bind_constructors => ["new"],
    );

}

sub bind_requiredoptionalquery {
    my $synopsis = <<'END_SYNOPSIS';
    my $foo_and_maybe_bar = Lucy::Search::RequiredOptionalQuery->new(
        required_query => $foo_query,
        optional_query => $bar_query,
    );
    my $hits = $searcher->hits( query => $foo_and_maybe_bar );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $reqopt_query = Lucy::Search::RequiredOptionalQuery->new(
        required_query => $foo_query,    # required
        optional_query => $bar_query,    # required
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::RequiredOptionalQuery",
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

}

sub bind_searcher {
    my $constructor = <<'END_CONSTRUCTOR';
    package MySearcher;
    use base qw( Lucy::Search::Searcher );
    sub new {
        my $self = shift->SUPER::new;
        ...
        return $self;
    }
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Searcher",
        bind_methods => [
            qw( Doc_Max
                Doc_Freq
                Glean_Query
                Hits
                Collect
                Top_Docs
                Fetch_Doc
                Fetch_Doc_Vec
                Get_Schema
                Close )
        ],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => "    # Abstract base class.\n",
            constructor => { sample => $constructor },
            methods     => [
                qw(
                    hits
                    collect
                    glean_query
                    doc_max
                    doc_freq
                    fetch_doc
                    get_schema
                    )
            ],
        },
    );

}

sub bind_sortrule {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Search::SortRule

int32_t
FIELD()
CODE:
    RETVAL = lucy_SortRule_FIELD;
OUTPUT: RETVAL

int32_t
SCORE()
CODE:
    RETVAL = lucy_SortRule_SCORE;
OUTPUT: RETVAL

int32_t
DOC_ID()
CODE:
    RETVAL = lucy_SortRule_DOC_ID;
OUTPUT: RETVAL
END_XS_CODE

    my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = Lucy::Search::SortSpec->new(
        rules => [
            Lucy::Search::SortRule->new( field => 'date' ),
            Lucy::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $by_title   = Lucy::Search::SortRule->new( field => 'title' );
    my $by_score   = Lucy::Search::SortRule->new( type  => 'score' );
    my $by_doc_id  = Lucy::Search::SortRule->new( type  => 'doc_id' );
    my $reverse_date = Lucy::Search::SortRule->new(
        field   => 'date',
        reverse => 1,
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::SortRule",
        xs_code           => $xs_code,
        bind_constructors => ["_new"],
        bind_methods      => [qw( Get_Field Get_Reverse )],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( get_field get_reverse )],
        },
    );

}

sub bind_sortspec {
    my $synopsis = <<'END_SYNOPSIS';
    my $sort_spec = Lucy::Search::SortSpec->new(
        rules => [
            Lucy::Search::SortRule->new( field => 'date' ),
            Lucy::Search::SortRule->new( type  => 'doc_id' ),
        ],
    );
    my $hits = $searcher->hits(
        query     => $query,
        sort_spec => $sort_spec,
    );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $sort_spec = Lucy::Search::SortSpec->new( rules => \@rules );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::SortSpec",
        bind_methods      => [qw( Get_Rules )],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );

}

sub bind_span {
    my $synopsis = <<'END_SYNOPSIS';
    my $combined_length = $upper_span->get_length
        + ( $upper_span->get_offset - $lower_span->get_offset );
    my $combined_span = Lucy::Search::Span->new(
        offset => $lower_span->get_offset,
        length => $combined_length,
    );
    ...
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $span = Lucy::Search::Span->new(
        offset => 75,     # required
        length => 7,      # required
        weight => 1.0,    # default 0.0
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Span",
        bind_methods => [
            qw( Set_Offset
                Get_Offset
                Set_Length
                Get_Length
                Set_Weight
                Get_Weight )
        ],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [
                qw( set_offset
                    get_offset
                    set_length
                    get_length
                    set_weight
                    get_weight )
            ],
        }
    );

}

sub bind_termquery {
    my $synopsis = <<'END_SYNOPSIS';
    my $term_query = Lucy::Search::TermQuery->new(
        field => 'content',
        term  => 'foo', 
    );
    my $hits = $searcher->hits( query => $term_query );
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR';
    my $term_query = Lucy::Search::TermQuery->new(
        field => 'content',    # required
        term  => 'foo',        # required
    );
END_CONSTRUCTOR

    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::TermQuery",
        bind_methods      => [qw( Get_Field Get_Term )],
        bind_constructors => ["new"],
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( get_field get_term )],
        },
    );
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::TermCompiler",
        bind_constructors => ["do_new"],
    );

}

sub bind_topdocs {
    Clownfish::CFC::Binding::Perl::Class->register(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::TopDocs",
        bind_methods => [
            qw(
                Get_Match_Docs
                Get_Total_Hits
                Set_Total_Hits
                )
        ],
        bind_constructors => ["new"],
    );

}

1;
