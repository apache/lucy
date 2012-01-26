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
use strict;
use warnings;

sub bind_all {
    my $class = shift;
    $class->bind_andmatcher;
    $class->bind_andquery;
    $class->bind_bitvecmatcher;
    $class->bind_collector;
    $class->bind_offsetcollector;
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
    $class->bind_phrasecompiler;
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
    $class->bind_termcompiler;
    $class->bind_topdocs;
}

sub bind_andmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ANDMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ANDQuery",
        make_pod          => {
            methods     => [qw( add_child )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_bitvecmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::BitVecMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Collector",
        make_pod          => {
            synopsis    => "    # Abstract base class.\n",
            constructor => { sample => $constructor },
            methods     => [qw( collect )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Collect
        Set_Reader
        Set_Base
        Set_Matcher
        Need_Score
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_offsetcollector {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::Collector::OffsetCollector",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Compiler",
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
    $binding->bind_constructor( alias => 'do_new' );
    $binding->bind_method( method => $_ ) for qw(
        Make_Matcher
        Get_Parent
        Get_Similarity
        Get_Weight
        Sum_Of_Squared_Weights
        Apply_Norm_Factor
        Normalize
        Highlight_Spans
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hitqueue {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::HitQueue",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Hits",
        make_pod          => {
            synopsis => $synopsis,
            methods  => [qw( next total_hits )],
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Total_Hits Next );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::IndexSearcher",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Reader );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::LeafQuery",
        make_pod          => {
            methods     => [qw( get_field get_text )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Field Get_Text );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matchallquery {
    my $constructor = <<'END_CONSTRUCTOR';
    my $match_all_query = Lucy::Search::MatchAllQuery->new;
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::MatchAllQuery",
        make_pod          => { constructor => { sample => $constructor }, }
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matchdoc {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::MatchDoc",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Doc_ID
        Set_Doc_ID
        Get_Score
        Set_Score
        Get_Values
        Set_Values
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matcher {
    my $synopsis = <<'END_SYNOPSIS';
    # abstract base class
END_SYNOPSIS

    my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $matcher = MyMatcher->SUPER::new;
END_CONSTRUCTOR_CODE_SAMPLE

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::Matcher",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( next advance get_doc_id score )],
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Next
        Advance
        Get_Doc_ID
        Score
        Collect
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_notmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NOTMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NOTQuery",
        make_pod          => {
            methods     => [qw( get_negated_query set_negated_query )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        }
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Negated_Query
        Set_Negated_Query
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_nomatchquery {
    my $constructor = <<'END_CONSTRUCTOR';
    my $no_match_query = Lucy::Search::NoMatchQuery->new;
END_CONSTRUCTOR

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::NoMatchQuery",
        make_pod          => { constructor => { sample => $constructor }, }
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ORQuery",
        make_pod          => {
            methods     => [qw( add_child )],
            synopsis    => $synopsis,
            constructor => { sample => $constructor, }
        },
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_orscorer {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::ORScorer",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_phrasequery {
    my $synopsis = <<'END_SYNOPSIS';
    my $phrase_query = Lucy::Search::PhraseQuery->new( 
        field => 'content',
        terms => [qw( the who )],
    );
    my $hits = $searcher->hits( query => $phrase_query );
END_SYNOPSIS

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PhraseQuery",
        make_pod          => {
            constructor => { sample => '' },
            synopsis    => $synopsis,
            methods     => [qw( get_field get_terms )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Field Get_Terms );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_phrasecompiler {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PhraseCompiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PolyQuery",
        make_pod          => { synopsis => $synopsis, },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Add_Child
        Set_Children
        Get_Children
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::PolySearcher",
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
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Query",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( make_compiler set_boost get_boost )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Set_Boost Get_Boost );
    $binding->bind_method(
        alias  => '_make_compiler',
        method => 'Make_Compiler',
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::QueryParser",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
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
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::RangeQuery",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_requiredoptionalmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::RequiredOptionalMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::RequiredOptionalQuery",
        make_pod          => {
            methods => [
                qw( get_required_query set_required_query
                    get_optional_query set_optional_query )
            ],
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Required_Query
        Set_Required_Query
        Get_Optional_Query
        Set_Optional_Query
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Searcher",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Doc_Max
        Doc_Freq
        Glean_Query
        Hits
        Collect
        Top_Docs
        Fetch_Doc
        Fetch_Doc_Vec
        Get_Schema
        Close
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::SortRule",
        xs_code           => $xs_code,
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( get_field get_reverse )],
        },
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->bind_method( method => $_ ) for qw( Get_Field Get_Reverse );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::SortSpec",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Rules );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::Span",
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
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Set_Offset
        Get_Offset
        Set_Length
        Get_Length
        Set_Weight
        Get_Weight
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::TermQuery",
        make_pod          => {
            synopsis    => $synopsis,
            constructor => { sample => $constructor },
            methods     => [qw( get_field get_term )],
        },
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw( Get_Field Get_Term );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_termcompiler {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel            => "Lucy",
        class_name        => "Lucy::Search::TermCompiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_topdocs {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel       => "Lucy",
        class_name   => "Lucy::Search::TopDocs",
    );
    $binding->bind_constructor;
    $binding->bind_method( method => $_ ) for qw(
        Get_Match_Docs
        Get_Total_Hits
        Set_Total_Hits
    );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
