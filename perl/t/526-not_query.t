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

use Test::More tests => 61;
use Storable qw( freeze thaw );
use Lucy::Test::TestUtils qw( create_index );
use LucyX::Search::MockMatcher;

my @got;

my $folder = create_index( 'a' .. 'z' );

my $b_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'b'
);
my $c_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'c'
);
my $not_b_query = Lucy::Search::NOTQuery->new( negated_query => $b_query );
my $not_c_query = Lucy::Search::NOTQuery->new( negated_query => $c_query );

is( $not_b_query->to_string, "-content:b", "to_string" );

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $reader   = $searcher->get_reader;
my $hits     = $searcher->hits(
    query      => $not_b_query,
    num_wanted => 100
);
is( $hits->total_hits, 25, "not b" );
@got = ();
while ( my $hit = $hits->next ) {
    push @got, $hit->{content};
}
is_deeply( \@got, [ 'a', 'c' .. 'z' ], "correct hits" );

my $frozen = freeze($not_b_query);
my $thawed = thaw($frozen);
ok( $not_b_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$not_b_query->equals($thawed), '!equals (boost)' );
ok( !$not_b_query->equals($not_c_query),
    "!equals (different negated query)" );

my $compiler = $not_b_query->make_compiler(
    searcher => $searcher,
    boost    => $not_b_query->get_boost,
);
$frozen = freeze($compiler);
$thawed = thaw($frozen);
ok( $thawed->equals($compiler), 'freeze/thaw compiler' );

# Air out NOTMatcher with random patterns.
for my $num_negated ( 1 .. 26 ) {
    my @source_ids = ( 1 .. 26 );
    my @mock_ids;
    for ( 1 .. $num_negated ) {
        my $tick = int( rand @source_ids );
        push @mock_ids, splice( @source_ids, $tick, 1 );
    }
    @mock_ids = sort { $a <=> $b } @mock_ids;
    my $mock_matcher = LucyX::Search::MockMatcher->new(
        doc_ids => \@mock_ids,
        scores  => [ (1) x scalar @mock_ids ],
    );
    my $not_matcher = Lucy::Search::NOTMatcher->new(
        doc_max         => $reader->doc_max,
        negated_matcher => $mock_matcher,
    );
    my $bit_vec = Lucy::Object::BitVector->new( capacity => 30 );
    my $collector
        = Lucy::Search::Collector::BitCollector->new( bit_vector => $bit_vec,
        );
    $not_matcher->collect( collector => $collector );
    my $got = $bit_vec->to_arrayref;
    is( scalar @$got, scalar @source_ids, "got all docs ($num_negated)" );
    is_deeply( $got, \@source_ids, "correct retrieval ($num_negated)" );
}

my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => Lucy::Test::TestSchema->new,
);
$indexer->delete_by_term( field => 'content', term => 'b' );
$indexer->commit;

@got      = ();
$searcher = Lucy::Search::IndexSearcher->new( index => $folder );
$hits     = $searcher->hits( query => $not_b_query, num_wanted => 100 );
is( $hits->total_hits, 25, "still correct after deletion" );
while ( my $hit = $hits->next ) {
    push @got, $hit->{content};
}
is_deeply( \@got, [ 'a', 'c' .. 'z' ], "correct hits after deletion" );
