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

use strict;
use warnings;

use lib 'buildlib';
use Lucy::Test;

package PlainSchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self      = shift->SUPER::new(@_);
    my $tokenizer = Lucy::Analysis::RegexTokenizer->new( pattern => '\S+' );
    my $type      = Lucy::Plan::FullTextType->new( analyzer => $tokenizer, );
    $self->spec_field( name => 'content', type => $type );
    return $self;
}

package StopSchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $whitespace_tokenizer
        = Lucy::Analysis::RegexTokenizer->new( token_re => qr/\S+/ );
    my $stopfilter
        = Lucy::Analysis::SnowballStopFilter->new( stoplist => { x => 1 } );
    my $polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $whitespace_tokenizer, $stopfilter, ], );
    my $type = Lucy::Plan::FullTextType->new( analyzer => $polyanalyzer, );
    $self->spec_field( name => 'content', type => $type );
    return $self;
}

package MyTermQuery;
use base qw( Lucy::Search::TermQuery );

package MyPhraseQuery;
use base qw( Lucy::Search::PhraseQuery );

package MyANDQuery;
use base qw( Lucy::Search::ANDQuery );

package MyORQuery;
use base qw( Lucy::Search::ORQuery );

package MyNOTQuery;
use base qw( Lucy::Search::NOTQuery );

package MyReqOptQuery;
use base qw( Lucy::Search::RequiredOptionalQuery );

package MyQueryParser;
use base qw( Lucy::Search::QueryParser );

sub make_term_query    { shift; MyTermQuery->new(@_) }
sub make_phrase_query  { shift; MyPhraseQuery->new(@_) }
sub make_and_query     { shift; MyANDQuery->new( children => shift ) }
sub make_or_query      { shift; MyORQuery->new( children => shift ) }
sub make_not_query     { shift; MyNOTQuery->new( negated_query => shift ) }
sub make_req_opt_query { shift; MyReqOptQuery->new(@_) }

package main;
use Test::More tests => 224;
use Clownfish::Util::StringHelper qw( utf8_flag_on utf8ify );
use Lucy::Test::TestUtils qw( create_index );

my $folder       = Lucy::Store::RAMFolder->new;
my $stop_folder  = Lucy::Store::RAMFolder->new;
my $plain_schema = PlainSchema->new;
my $stop_schema  = StopSchema->new;

my @docs = ( 'x', 'y', 'z', 'x a', 'x a b', 'x a b c', 'x foo a b c d', );
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $plain_schema,
);
my $stop_indexer = Lucy::Index::Indexer->new(
    index  => $stop_folder,
    schema => $stop_schema,
);

for (@docs) {
    $indexer->add_doc( { content => $_ } );
    $stop_indexer->add_doc( { content => $_ } );
}
$indexer->commit;
$stop_indexer->commit;

my $OR_parser = Lucy::Search::QueryParser->new( schema => $plain_schema, );
my $AND_parser = Lucy::Search::QueryParser->new(
    schema         => $plain_schema,
    default_boolop => 'AND',
);
$OR_parser->set_heed_colons(1);
$AND_parser->set_heed_colons(1);

my $OR_stop_parser
    = Lucy::Search::QueryParser->new( schema => $stop_schema, );
my $AND_stop_parser = Lucy::Search::QueryParser->new(
    schema         => $stop_schema,
    default_boolop => 'AND',
);
$OR_stop_parser->set_heed_colons(1);
$AND_stop_parser->set_heed_colons(1);

my $searcher      = Lucy::Search::IndexSearcher->new( index => $folder );
my $stop_searcher = Lucy::Search::IndexSearcher->new( index => $stop_folder );

my @logical_tests = (

    'b'     => [ 3, 3, 3, 3, ],
    '(a)'   => [ 4, 4, 4, 4, ],
    '"a"'   => [ 4, 4, 4, 4, ],
    '"(a)"' => [ 0, 0, 0, 0, ],
    '("a")' => [ 4, 4, 4, 4, ],

    'a b'     => [ 4, 3, 4, 3, ],
    'a (b)'   => [ 4, 3, 4, 3, ],
    'a "b"'   => [ 4, 3, 4, 3, ],
    'a ("b")' => [ 4, 3, 4, 3, ],
    'a "(b)"' => [ 4, 0, 4, 0, ],

    '(a b)'   => [ 4, 3, 4, 3, ],
    '"a b"'   => [ 3, 3, 3, 3, ],
    '("a b")' => [ 3, 3, 3, 3, ],
    '"(a b)"' => [ 0, 0, 0, 0, ],

    'a b c'     => [ 4, 2, 4, 2, ],
    'a (b c)'   => [ 4, 2, 4, 2, ],
    'a "b c"'   => [ 4, 2, 4, 2, ],
    'a ("b c")' => [ 4, 2, 4, 2, ],
    'a "(b c)"' => [ 4, 0, 4, 0, ],
    '"a b c"'   => [ 2, 2, 2, 2, ],

    '-x'     => [ 0, 0, 0, 0, ],
    'x -c'   => [ 3, 3, 0, 0, ],
    'x "-c"' => [ 5, 0, 0, 0, ],
    'x +c'   => [ 2, 2, 2, 2, ],
    'x "+c"' => [ 5, 0, 0, 0, ],

    '+x +c' => [ 2, 2, 2, 2, ],
    '+x -c' => [ 3, 3, 0, 0, ],
    '-x +c' => [ 0, 0, 2, 2, ],
    '-x -c' => [ 0, 0, 0, 0, ],

    'x y'     => [ 6, 0, 1, 1, ],
    'x a d'   => [ 5, 1, 4, 1, ],
    'x "a d"' => [ 5, 0, 0, 0, ],
    '"x a"'   => [ 3, 3, 4, 4, ],

    'x AND y'     => [ 0, 0, 1, 1, ],
    'x OR y'      => [ 6, 6, 1, 1, ],
    'x AND NOT y' => [ 5, 5, 0, 0, ],

    'x (b OR c)'     => [ 5, 3, 3, 3, ],
    'x AND (b OR c)' => [ 3, 3, 3, 3, ],
    'x OR (b OR c)'  => [ 5, 5, 3, 3, ],
    'x (y OR c)'     => [ 6, 2, 3, 3, ],
    'x AND (y OR c)' => [ 2, 2, 3, 3, ],

    'a AND NOT (b OR "c d")'     => [ 1, 1, 1, 1, ],
    'a AND NOT "a b"'            => [ 1, 1, 1, 1, ],
    'a AND NOT ("a b" OR "c d")' => [ 1, 1, 1, 1, ],

    '+"b c" -d' => [ 1, 1, 1, 1, ],
    '"a b" +d'  => [ 1, 1, 1, 1, ],

    'x AND NOT (b OR (c AND d))' => [ 2, 2, 0, 0, ],

    '-(+notthere)' => [ 0, 0, 0, 0 ],

    'content:b'              => [ 3, 3, 3, 3, ],
    'bogusfield:a'           => [ 0, 0, 0, 0, ],
    'bogusfield:a content:b' => [ 3, 0, 3, 0, ],

    'content:b content:c' => [ 3, 2, 3, 2 ],
    'content:(b c)'       => [ 3, 2, 3, 2 ],
    'bogusfield:(b c)'    => [ 0, 0, 0, 0 ],

);

my $i = 0;
while ( $i < @logical_tests ) {
    my $qstring = $logical_tests[$i];
    $i++;

    my $query = $OR_parser->parse($qstring);
    my $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $logical_tests[$i][0], "OR:    $qstring" );

    $query = $AND_parser->parse($qstring);
    $hits = $searcher->hits( query => $query );
    is( $hits->total_hits, $logical_tests[$i][1], "AND:   $qstring" );

    $query = $OR_stop_parser->parse($qstring);
    $hits = $stop_searcher->hits( query => $query );
    is( $hits->total_hits, $logical_tests[$i][2], "stoplist-OR:   $qstring" );

    $query = $AND_stop_parser->parse($qstring);
    $hits = $stop_searcher->hits( query => $query );
    is( $hits->total_hits, $logical_tests[$i][3],
        "stoplist-AND:   $qstring" );

    $i++;
}

my $motorhead = "Mot\xF6rhead";
utf8ify($motorhead);
my $unicode_folder = create_index($motorhead);
$searcher = Lucy::Search::IndexSearcher->new( index => $unicode_folder );

my $hits = $searcher->hits( query => 'Mot' );
is( $hits->total_hits, 0, "Pre-test - indexing worked properly" );
$hits = $searcher->hits( query => $motorhead );
is( $hits->total_hits, 1, "QueryParser parses UTF-8 strings correctly" );

my $custom_parser = MyQueryParser->new( schema => PlainSchema->new );
isa_ok( $custom_parser->parse('foo'),         'MyTermQuery' );
isa_ok( $custom_parser->parse('"foo bar"'),   'MyPhraseQuery' );
isa_ok( $custom_parser->parse('foo AND bar'), 'MyANDQuery' );
isa_ok( $custom_parser->parse('foo OR bar'),  'MyORQuery' );
isa_ok( $custom_parser->tree('NOT foo'),      'MyNOTQuery' );
isa_ok( $custom_parser->parse('+foo bar'),    'MyReqOptQuery' );
