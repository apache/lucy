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

our $VERSION = '0.004002';
$VERSION = eval $VERSION;

sub bind_all {
    my $class = shift;
    $class->bind_andquery;
    $class->bind_collector;
    $class->bind_bitcollector;
    $class->bind_compiler;
    $class->bind_hits;
    $class->bind_indexsearcher;
    $class->bind_leafquery;
    $class->bind_matchallquery;
    $class->bind_matcher;
    $class->bind_notquery;
    $class->bind_nomatchquery;
    $class->bind_orquery;
    $class->bind_parserelem;
    $class->bind_phrasequery;
    $class->bind_phrasecompiler;
    $class->bind_polyquery;
    $class->bind_polysearcher;
    $class->bind_query;
    $class->bind_queryparser;
    $class->bind_rangequery;
    $class->bind_requiredoptionalquery;
    $class->bind_searcher;
    $class->bind_sortrule;
    $class->bind_sortspec;
    $class->bind_span;
    $class->bind_termquery;
    $class->bind_termcompiler;
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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_collector {
    my @exposed = qw( Collect );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_bitcollector {
    my @exposed = qw( Collect );

    my $pod_spec = Clownfish::CFC::Binding::Perl::Pod->new;
    my $synopsis = <<'END_SYNOPSIS';
    my $bit_vec = Lucy::Object::BitVector->new(
        capacity => $searcher->doc_max + 1,
    );
    my $bit_collector = Lucy::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec, 
    );
    $searcher->collect(
        collector => $bit_collector,
        query     => $query,
    );
END_SYNOPSIS
    my $constructor = <<'END_CONSTRUCTOR';
    my $bit_collector = Lucy::Search::Collector::BitCollector->new(
        bit_vector => $bit_vec,    # required
    );
END_CONSTRUCTOR
    $pod_spec->set_synopsis($synopsis);
    $pod_spec->add_constructor( alias => 'new', sample => $constructor, );
    $pod_spec->add_method( method => $_, alias => lc($_) ) for @exposed;

    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::Collector::BitCollector",
    );
    $binding->set_pod_spec($pod_spec);

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

sub bind_hits {
    my @exposed = qw( Next Total_Hits );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_indexsearcher {
    my @exposed = qw(
        Hits
        Collect
        Doc_Max
        Doc_Freq
        Fetch_Doc
        Get_Schema
        Get_Reader
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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_leafquery {
    my @exposed = qw( Get_Field Get_Text );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_matcher {
    my @exposed = qw(
        Next
        Advance
        Get_Doc_ID
        Score
    );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_notquery {
    my @exposed = qw(
        Get_Negated_Query
        Set_Negated_Query
    );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_parserelem {
    my $xs_code = <<'END_XS_CODE';
MODULE = Lucy   PACKAGE = Lucy::Search::QueryParser::ParserElem

SV*
new(either_sv, ...)
    SV *either_sv;
CODE:
{
    SV              *type_sv  = NULL;
    SV              *value_sv = NULL;
    const char      *type_str = NULL;
    cfish_Obj       *value    = NULL;
    uint32_t         type     = 0;
    lucy_ParserElem *self     = NULL;
    bool args_ok;

    args_ok
        = XSBind_allot_params(&(ST(0)), 1, items,
                              ALLOT_SV(&type_sv, "type", 4, true),
                              ALLOT_SV(&value_sv, "value", 5, false),
                              NULL);

    if (!args_ok) {
        CFISH_RETHROW(CFISH_INCREF(cfish_Err_get_error()));
    }
    type_str = SvPVutf8_nolen(type_sv);

    if (strcmp(type_str, "OPEN_PAREN") == 0) {
        type = LUCY_QPARSER_TOKEN_OPEN_PAREN; 
    }
    else if (strcmp(type_str, "CLOSE_PAREN") == 0) {
        type = LUCY_QPARSER_TOKEN_CLOSE_PAREN; 
    }
    else if (strcmp(type_str, "MINUS") == 0) {
        type = LUCY_QPARSER_TOKEN_MINUS; 
    }
    else if (strcmp(type_str, "PLUS") == 0) {
        type = LUCY_QPARSER_TOKEN_PLUS; 
    }
    else if (strcmp(type_str, "NOT") == 0) {
        type = LUCY_QPARSER_TOKEN_NOT; 
    }
    else if (strcmp(type_str, "AND") == 0) {
        type = LUCY_QPARSER_TOKEN_AND; 
    }
    else if (strcmp(type_str, "OR") == 0) {
        type = LUCY_QPARSER_TOKEN_OR; 
    }
    else if (strcmp(type_str, "FIELD") == 0) {
        type = LUCY_QPARSER_TOKEN_FIELD; 
        value = CFISH_CERTIFY(XSBind_perl_to_cfish(value_sv), CFISH_STRING);
    }
    else if (strcmp(type_str, "STRING") == 0) {
        type = LUCY_QPARSER_TOKEN_STRING; 
        value = CFISH_CERTIFY(XSBind_perl_to_cfish(value_sv), CFISH_STRING);
    }
    else if (strcmp(type_str, "QUERY") == 0) {
        type = LUCY_QPARSER_TOKEN_QUERY; 
        value = CFISH_CERTIFY(XSBind_perl_to_cfish(value_sv), LUCY_QUERY);
    }
    else {
        CFISH_THROW(CFISH_ERR, "Bad type: '%s'", type_str);
    }

    self = (lucy_ParserElem*)XSBind_new_blank_obj(either_sv);
    self = lucy_ParserElem_init(self, type, value);
    RETVAL = XSBind_cfish_to_perl((cfish_Obj*)self);
    CFISH_DECREF(self);
}
OUTPUT: RETVAL

END_XS_CODE
    my $binding = Clownfish::CFC::Binding::Perl::Class->new(
        parcel     => "Lucy",
        class_name => "Lucy::Search::QueryParser::ParserElem",
    );
    $binding->exclude_constructor;
    $binding->append_xs($xs_code);
    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_phrasequery {
    my @exposed = qw( Get_Field Get_Terms );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_query {
    my @exposed = qw( Make_Compiler Set_Boost Get_Boost );

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
    $binding->bind_method(
        alias  => '_load',
        method => 'Load',
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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_requiredoptionalquery {
    my @exposed = qw(
        Get_Required_Query
        Set_Required_Query
        Get_Optional_Query
        Set_Optional_Query
    );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_sortrule {
    my @exposed = qw( Get_Field Get_Reverse );

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
    $binding->set_pod_spec($pod_spec);

    Clownfish::CFC::Binding::Perl::Class->register($binding);
}

sub bind_termquery {
    my @exposed = qw( Get_Field Get_Term );

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

1;
