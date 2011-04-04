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

use Test::More tests => 1362;
use Lucy::Test;
use LucyX::Search::MockMatcher;
use Lucy::Test::TestUtils qw( modulo_set doc_ids_from_td_coll );

my $sim = Lucy::Index::Similarity->new;

for my $interval_a ( reverse 1 .. 17 ) {
    for my $interval_b ( reverse 10 .. 17 ) {
        check_matcher( $interval_a, $interval_b );
        for my $interval_c ( 30, 75 ) {
            check_matcher( $interval_a, $interval_b, $interval_c );
            check_matcher( $interval_c, $interval_b, $interval_a );
        }
    }
}
check_matcher(1000);

sub check_matcher {
    my @intervals     = @_;
    my @doc_id_arrays = map { modulo_set( $_, 100 ) } @intervals;
    my @children      = map {
        LucyX::Search::MockMatcher->new(
            doc_ids => $_,
            scores  => [ (0) x scalar @$_ ],
            )
    } @doc_id_arrays;
    my $and_matcher = Lucy::Search::ANDMatcher->new(
        children   => \@children,
        similarity => $sim,
    );
    my @expected = intersect(@doc_id_arrays);
    my $collector
        = Lucy::Search::Collector::SortCollector->new( wanted => 1000 );
    $and_matcher->collect( collector => $collector );
    is( $collector->get_total_hits,
        scalar @expected,
        "correct num hits @intervals"
    );
    my ( $by_score, $by_id ) = doc_ids_from_td_coll($collector);
    is_deeply( $by_id, \@expected, "correct doc nums @intervals" );
}

sub intersect {
    my @arrays = @_;
    my @out    = @{ $arrays[0] };
    for my $array (@arrays) {
        my %hash;
        @hash{@$array} = (1) x @$array;
        @out = grep { exists $hash{$_} } @out;
    }
    return @out;
}

# Trigger destruction.
undef $sim;
