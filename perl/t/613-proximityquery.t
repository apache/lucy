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

use Test::More tests => 11;
use Storable qw( freeze thaw );
use Lucy::Test;
use Lucy::Test::TestUtils qw( create_index );
use LucyX::Search::ProximityQuery;

# this is better than 'x a b c d a b c d' because its
# posting weight is higher, presumably because
# it is a shorter doc (higher density?)
my $best_match = 'a b c d x x a';

my @docs = (
    1 .. 20,
    'a b c a b c a b c d',
    'x a b c d a b c d',
    'a c b d', 'a x x x b x x x c x x x x x x d x',
    $best_match, 'a' .. 'z',
);

my $folder = create_index(@docs);
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [],
    within => 10,
);
is( $proximity_query->to_string, 'content:""~10',
    "empty ProximityQuery to_string" );
$proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( d a )],
    within => 10,
);
is( $proximity_query->to_string, 'content:"d a"~10', "to_string" );

my $hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 2, "correct number of hits" );
my $first_hit = $hits->next;
is( $first_hit->{content}, $best_match, 'best match appears first' );

my $second_hit = $hits->next;
ok( $first_hit->get_score > $second_hit->get_score,
    "best match scores higher: "
        . $first_hit->get_score . " > "
        . $second_hit->get_score
);

$proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( c a )],
    within => 10,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 3, 'avoid underflow when subtracting offset' );

$proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( b d )],
    within => 10,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 4, 'offset starting from zero' );

my $frozen = freeze($proximity_query);
my $thawed = thaw($frozen);
$hits = $searcher->hits( query => $thawed );
is( $hits->total_hits, 4, 'freeze/thaw' );

my $proximity_compiler
    = $proximity_query->make_compiler( searcher => $searcher, );
$frozen = freeze($proximity_compiler);
$thawed = thaw($frozen);
ok( $proximity_compiler->equals($thawed), "freeze/thaw compiler" );

$proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( x d )],
    within => 4,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 2, 'within range is exclusive' );
$proximity_query = LucyX::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( x d )],
    within => 3,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 1, 'within range is exclusive' );
