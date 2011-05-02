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

use Test::More tests => 5;
use Lucy::Test;
use LucyX::Search::MockMatcher;

my @docs   = ( 1, 5, 10, 1000 );
my @scores = ( 2, 0, 0,  1 );

my $collector = Lucy::Search::Collector::SortCollector->new( wanted => 3 );
test_collect($collector);

my @got = map { $_->get_score } @{ $collector->pop_match_docs };
is_deeply( \@got, [ 2, 1, 0 ], "collect into HitQueue" );

$collector = Lucy::Search::Collector::SortCollector->new( wanted => 0 );
test_collect($collector);
is( $collector->get_total_hits, 4,
    "get_total_hits is accurate when no hits are requested" );
my $match_docs = $collector->pop_match_docs;
is( scalar @$match_docs, 0, "no hits wanted, so no hits returned" );

my $bit_vec = Lucy::Object::BitVector->new;
$collector
    = Lucy::Search::Collector::BitCollector->new( bit_vector => $bit_vec );
test_collect($collector);
is_deeply(
    $bit_vec->to_arrayref,
    [ 1, 5, 10, 1000 ],
    "BitCollector collects the right doc nums"
);

$bit_vec = Lucy::Object::BitVector->new;
my $inner_coll
    = Lucy::Search::Collector::BitCollector->new( bit_vector => $bit_vec );
my $offset_coll = Lucy::Search::Collector::OffsetCollector->new(
    collector => $inner_coll,
    offset    => 10,
);
test_collect($offset_coll);
is_deeply( $bit_vec->to_arrayref, [ 11, 15, 20, 1010 ], "Offset collector" );

sub test_collect {
    my $collector = shift;
    my $matcher   = LucyX::Search::MockMatcher->new(
        doc_ids => \@docs,
        scores  => \@scores,
    );
    $collector->set_matcher($matcher);
    while ( my $doc_id = $matcher->next ) {
        $collector->collect($doc_id);
    }
}
