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

use Test::More tests => 900;
use Lucy::Test;
use LucyX::Search::MockMatcher;
use Lucy::Test::TestUtils qw( modulo_set doc_ids_from_td_coll );

my $sim = Lucy::Index::Similarity->new;

for my $interval_a ( 1 .. 10 ) {
    for my $interval_b ( 5 .. 10 ) {
        check_matcher( $interval_a, $interval_b );
        for my $interval_c ( 30, 75 ) {
            check_matcher( $interval_a, $interval_b, $interval_c );
            check_matcher( $interval_c, $interval_b, $interval_a );
        }
    }
}

sub check_matcher {
    my @intervals = @_;
    my @doc_id_arrays = map { modulo_set( $_, 100 ) } @intervals;
    my $child_matchers
        = Lucy::Object::VArray->new( capacity => scalar @intervals );
    for my $doc_id_array (@doc_id_arrays) {
        my $mock = LucyX::Search::MockMatcher->new(
            doc_ids => $doc_id_array,
            scores  => [ (1) x scalar @$doc_id_array ],
        );
        $child_matchers->push($mock);
    }

    my $or_scorer = Lucy::Search::ORScorer->new(
        similarity => $sim,
        children   => $child_matchers,
    );
    my $collector
        = Lucy::Search::Collector::SortCollector->new( wanted => 100 );
    $or_scorer->collect( collector => $collector );
    my ( $got_by_score, $got_by_id ) = doc_ids_from_td_coll($collector);
    my ( $expected_by_count, $expected_by_id )
        = union_doc_id_sets(@doc_id_arrays);
    is( scalar @$got_by_id,
        scalar @$expected_by_id,
        "total hits: @intervals"
    );
    is_deeply( $got_by_id, $expected_by_id, "got all docs: @intervals" );
    is_deeply( $got_by_score, $expected_by_count,
        "scores accumulated: @intervals" );
}

sub union_doc_id_sets {
    my @arrays = @_;
    my %scores;
    for my $array (@arrays) {
        $scores{$_} += 1 for @$array;
    }
    my @by_count_then_id = sort { $scores{$b} <=> $scores{$a} or $a <=> $b }
        keys %scores;
    my @by_id = sort { $a <=> $b } keys %scores;
    return ( \@by_count_then_id, \@by_id );
}

# Trigger destruction.
undef $sim;
