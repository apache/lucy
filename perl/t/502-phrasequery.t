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

use Test::More tests => 9;
use Storable qw( freeze thaw );
use Lucy::Test;
use Lucy::Test::TestUtils qw( create_index );

my $best_match = 'x a b c d a b c d';

my @docs = (
    1 .. 20,
    'a b c a b c a b c d',
    'a b c d x x a',
    'a c b d', 'a x x x b x x x c x x x x x x d x',
    $best_match, 'a' .. 'z',
);

my $folder = create_index(@docs);
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $phrase_query = Lucy::Search::PhraseQuery->new(
    field => 'content',
    terms => [],
);
is( $phrase_query->to_string, 'content:""', "empty PhraseQuery to_string" );
$phrase_query = Lucy::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( a b c d )],
);
is( $phrase_query->to_string, 'content:"a b c d"', "to_string" );

my $hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 3, "correct number of hits" );
my $first_hit = $hits->next;
is( $first_hit->{content}, $best_match, 'best match appears first' );

my $second_hit = $hits->next;
ok( $first_hit->get_score > $second_hit->get_score,
    "best match scores higher: "
        . $first_hit->get_score . " > "
        . $second_hit->get_score
);

$phrase_query = Lucy::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( c a )],
);
$hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 1, 'avoid underflow when subtracting offset' );

# "a b c"
$phrase_query = Lucy::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( a b c )],
);
$hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 3, 'offset starting from zero' );

my $frozen = freeze($phrase_query);
my $thawed = thaw($frozen);
$hits = $searcher->hits( query => $thawed );
is( $hits->total_hits, 3, 'freeze/thaw' );

my $phrase_compiler = $phrase_query->make_compiler(
    searcher => $searcher,
    boost    => $phrase_query->get_boost,
);
$frozen = freeze($phrase_compiler);
$thawed = thaw($frozen);
ok( $phrase_compiler->equals($thawed), "freeze/thaw compiler" );
