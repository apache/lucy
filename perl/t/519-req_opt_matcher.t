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

use Test::More tests => 792;
use Lucy::Test;
use LucyX::Search::MockMatcher;
use Lucy::Test::TestUtils qw( modulo_set doc_ids_from_td_coll );

my $sim = Lucy::Index::Similarity->new;

for my $req_interval ( 1 .. 10, 75 ) {
    for my $opt_interval ( 1 .. 10, 75, 1000 ) {    # 1000 = no matches
        check_matcher( $req_interval, $opt_interval );
        check_matcher( $opt_interval, $req_interval );
    }
}

sub check_matcher {
    my ( $req_interval, $opt_interval ) = @_;
    my $req_docs = modulo_set( $req_interval, 100 );
    my $opt_docs = modulo_set( $opt_interval, 100 );
    my $req_mock = LucyX::Search::MockMatcher->new(
        doc_ids => $req_docs,
        scores  => [ (1) x scalar @$req_docs ],
    );
    my $opt_mock;
    if (@$opt_docs) {
        $opt_mock = LucyX::Search::MockMatcher->new(
            doc_ids => $opt_docs,
            scores  => [ (1) x scalar @$opt_docs ],
        );
    }
    my $req_opt_matcher = Lucy::Search::RequiredOptionalMatcher->new(
        similarity       => $sim,
        required_matcher => $req_mock,
        optional_matcher => $opt_mock,
    );
    my $collector
        = Lucy::Search::Collector::SortCollector->new( wanted => 1000 );
    $req_opt_matcher->collect( collector => $collector );
    my ( $got_by_score, $got_by_id ) = doc_ids_from_td_coll($collector);
    my ( $expected_by_count, $expected_by_id )
        = calc_result_sets( $req_interval, $opt_interval );
    is( scalar @$got_by_id,
        scalar @$expected_by_id,
        "total hits: $req_interval $opt_interval"
    );

    is_deeply( $got_by_id, $expected_by_id,
        "got all docs: $req_interval $opt_interval" );

    is_deeply( $got_by_score, $expected_by_count,
        "scores accumulated: $req_interval $opt_interval" );
}

sub calc_result_sets {
    my ( $req_interval, $opt_interval ) = @_;

    my @good;
    my @better;
    for my $doc_id ( 1 .. 99 ) {
        if ( $doc_id % $req_interval == 0 ) {
            if ( $doc_id % $opt_interval == 0 ) {
                push @better, $doc_id;
            }
            else {
                push @good, $doc_id;
            }
        }
    }
    my @by_count_then_id = ( @better, @good );
    my @by_id = sort { $a <=> $b } @by_count_then_id;

    return ( \@by_count_then_id, \@by_id );
}

# Trigger destruction.
undef $sim;
