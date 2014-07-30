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

use Test::More tests => 7;
use Storable qw( freeze thaw );
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a' .. 'z' );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $match_all_query = Lucy::Search::MatchAllQuery->new;
is( $match_all_query->to_string, "[MATCHALL]", "to_string" );

my $hits = $searcher->hits( query => $match_all_query );
is( $hits->total_hits, 26, "match all" );

my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => Lucy::Test::TestSchema->new,
);
$indexer->delete_by_term( field => 'content', term => 'b' );
$indexer->commit;

$searcher = Lucy::Search::IndexSearcher->new( index => $folder );
$hits = $searcher->hits( query => $match_all_query, num_wanted => 100 );
is( $hits->total_hits, 25, "match all minus a deletion" );
my @got;
while ( my $hit = $hits->next ) {
    push @got, $hit->{content};
}
is_deeply( \@got, [ 'a', 'c' .. 'z' ], "correct hits" );

my $frozen = freeze($match_all_query);
my $thawed = thaw($frozen);
ok( $match_all_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$match_all_query->equals($thawed), '!equals (boost)' );

my $compiler = $match_all_query->make_compiler(
    searcher => $searcher,
    boost    => $match_all_query->get_boost,
);
$frozen = freeze($compiler);
$thawed = thaw($frozen);
ok( $thawed->equals($compiler), "freeze/thaw compiler" );

