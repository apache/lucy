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
        parcel     => "Lucy",
        class_name => "Lucy::Search::ANDMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_andquery {
    my @exposed = qw( Add_Child );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::ANDQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_bitvecmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::BitVecMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_collector {
    my @exposed = qw( Collect );
    my @bound   = (
        @exposed,
        qw(
            Set_Reader
            Set_Base
            Set_Matcher
            Need_Score
            )
    );

    my $pod_spec    = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis("    # Abstract base class.\n");
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Collector",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_offsetcollector {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Collector::OffsetCollector",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_compiler {
    my @exposed = qw(
        Make_Matcher
        Get_Weight
        Sum_Of_Squared_Weights
        Apply_Norm_Factor
        Normalize
        Get_Parent
        Get_Similarity
        Highlight_Spans
    );
    my @bound = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Compiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hitqueue {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::HitQueue",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_hits {
    my @exposed = qw( Next Total_Hits );
    my @bound   = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Hits",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexsearcher {
    my @bound   = qw( Get_Reader );
    my @exposed = (
        qw(
            Hits
            Collect
            Doc_Max
            Doc_Freq
            Fetch_Doc
            Get_Schema
            ),
        @bound
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::IndexSearcher",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_leafquery {
    my @exposed = qw( Get_Field Get_Text );
    my @bound   = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::LeafQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matchallquery {
    my $pod_spec    = Clownfish::CFC::Binding::Perl::Pod->new;
    my $constructor = <<'END_CONSTRUCTOR';
    my $match_all_query = Lucy::Search::MatchAllQuery->new;
END_CONSTRUCTOR
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::MatchAllQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matchdoc {
    my @bound = qw(
        Get_Doc_ID
        Set_Doc_ID
        Get_Score
        Set_Score
        Get_Values
        Set_Values
    );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::MatchDoc",
    );
    $binding->bind_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matcher {
    my @exposed = qw(
        Next
        Advance
        Get_Doc_ID
        Score
    );
    my @bound = ( @exposed, 'Collect' );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    # abstract base class
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR_CODE_SAMPLE';
    my $matcher = MyMatcher->SUPER::new;
END_CONSTRUCTOR_CODE_SAMPLE
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Matcher",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_notmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::NOTMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_notquery {
    my @exposed = qw(
        Get_Negated_Query
        Set_Negated_Query
    );
    my @bound = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::NOTQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_nomatchquery {
    my $pod_spec    = Clownfish::CFC::Binding::Perl::Pod->new;
    my $constructor = <<'END_CONSTRUCTOR';
    my $no_match_query = Lucy::Search::NoMatchQuery->new;
END_CONSTRUCTOR
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::NoMatchQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_orquery {
    my @exposed = qw( Add_Child );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::ORQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_orscorer {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::ORScorer",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_phrasequery {
    my @exposed = qw( Get_Field Get_Terms );
    my @bound   = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $phrase_query = Lucy::Search::PhraseQuery->new( 
        field => 'content',
        terms => [qw( the who )],
    );
    my $hits = $searcher->hits( query => $phrase_query );
END_SYNOPSIS
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new' );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::PhraseQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_phrasecompiler {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::PhraseCompiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polyquery {
    my @bound = qw(
        Add_Child
        Set_Children
        Get_Children
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::PolyQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_polysearcher {
    my @exposed = qw(
        Hits
        Doc_Max
        Doc_Freq
        Fetch_Doc
        Get_Schema
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::PolySearcher",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_query {
    my @bound = qw( Set_Boost Get_Boost );
    my @exposed = ( 'Make_Compiler', @bound );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Query",
    );
    $binding->bind_constructor;
    $binding->bind_method(
        alias  => '_make_compiler',
        method => 'Make_Compiler',
    );
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_queryparser {
    my @exposed = qw(
        Parse
        Tree
        Expand
        Expand_Leaf
        Prune
        Set_Heed_Colons
        Make_Term_Query
        Make_Phrase_Query
        Make_AND_Query
        Make_OR_Query
        Make_NOT_Query
        Make_Req_Opt_Query
    );
    my @bound = (
        @exposed,
        qw(
            Get_Analyzer
            Get_Schema
            Get_Fields
            Heed_Colons
            )
    );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::QueryParser",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_rangequery {
    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::RangeQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_requiredoptionalmatcher {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::RequiredOptionalMatcher",
    );
    $binding->bind_constructor;
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_requiredoptionalquery {
    my @exposed = qw(
        Get_Required_Query
        Set_Required_Query
        Get_Optional_Query
        Set_Optional_Query
    );
    my @bound = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::RequiredOptionalQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_searcher {
    my @exposed = qw(
        Hits
        Collect
        Glean_Query
        Doc_Max
        Doc_Freq
        Fetch_Doc
        Get_Schema
    );
    my @bound = (
        @exposed,
        qw(
            Top_Docs
            Fetch_Doc_Vec
            Close
            )
    );

    my $pod_spec    = Clownfish::CFC::Binding::Perl::Pod->new;
    my $constructor = <<'END_CONSTRUCTOR';
    package MySearcher;
    use base qw( Lucy::Search::Searcher );
    sub new {
        my $self = shift->SUPER::new;
        ...
        return $self;
    }
END_CONSTRUCTOR
    $pod_spec->set_synopsis("    # Abstract base class.\n");
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Searcher",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortrule {
    my @exposed = qw( Get_Field Get_Reverse );
    my @bound   = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

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

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::SortRule",
    );
    $binding->bind_constructor( alias => '_new' );
    $binding->append_xs($xs_code);
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortspec {
    my @bound = qw( Get_Rules );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::SortSpec",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_span {
    my @exposed = qw(
        Set_Offset
        Get_Offset
        Set_Length
        Get_Length
        Set_Weight
        Get_Weight
    );
    my @bound = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Span",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_termquery {
    my @exposed = qw( Get_Field Get_Term );
    my @bound   = @exposed;

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
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
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::TermQuery",
    );
    $binding->bind_constructor;
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_termcompiler {
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::TermCompiler",
    );
    $binding->bind_constructor( alias => 'do_new' );
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_topdocs {
    my @bound = qw(
        Get_Match_Docs
        Get_Total_Hits
        Set_Total_Hits
    );

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::TopDocs",
    );
    $binding->bind_constructor;

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

1;
