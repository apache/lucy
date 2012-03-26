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

use Test::More tests => 8;
use Lucy::Test;
use Lucy::Test::TestUtils qw( create_index );

my $folder_a = create_index( 'x a', 'x b', 'x c' );
my $folder_b = create_index( 'y b', 'y c', 'y d' );
my $searcher_a = Lucy::Search::IndexSearcher->new( index => $folder_a );
my $searcher_b = Lucy::Search::IndexSearcher->new( index => $folder_b );

my $poly_searcher = Lucy::Search::PolySearcher->new(
    schema    => Lucy::Test::TestSchema->new,
    searchers => [ $searcher_a, $searcher_b ],
);

is( $poly_searcher->doc_freq( field => 'content', term => 'b' ),
    2, 'doc_freq' );
is( $poly_searcher->doc_max,                 6,     'doc_max' );
is( $poly_searcher->fetch_doc(1)->{content}, 'x a', "fetch_doc" );
isa_ok( $poly_searcher->fetch_doc_vec(1), 'Lucy::Index::DocVector' );

my $hits = $poly_searcher->hits( query => 'a' );
is( $hits->total_hits, 1, "Find hit in first searcher" );

$hits = $poly_searcher->hits( query => 'd' );
is( $hits->total_hits, 1, "Find hit in second searcher" );

$hits = $poly_searcher->hits( query => 'c' );
is( $hits->total_hits, 2, "Find hits in both searchers" );

my $bit_vec
    = Lucy::Object::BitVector->new( capacity => $poly_searcher->doc_max );
my $bitcoll
    = Lucy::Search::Collector::BitCollector->new( bit_vector => $bit_vec );
my $query = $poly_searcher->glean_query('b');
$poly_searcher->collect( query => $query, collector => $bitcoll );
is_deeply( $bit_vec->to_arrayref, [ 2, 4 ], "collect" );
