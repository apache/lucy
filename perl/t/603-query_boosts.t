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

use Test::More tests => 2;
use Lucy::Test::TestUtils qw( create_index );

my $doc_1
    = 'a a a a a a a a a a a a a a a a a a a b c d x y ' . ( 'z ' x 100 );
my $doc_2 = 'a b c d x y x y ' . ( 'z ' x 100 );

my $folder = create_index( $doc_1, $doc_2 );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $a_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'a',
);
my $x_y_query = Lucy::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( x y )],
);

my $combined_query
    = Lucy::Search::ORQuery->new( children => [ $a_query, $x_y_query ], );
my $hits = $searcher->hits( query => $combined_query );
my $hit = $hits->next;
is( $hit->{content}, $doc_1, "best doc ranks highest with no boosting" );

$x_y_query->set_boost(2);
$hits = $searcher->hits( query => $combined_query );
$hit = $hits->next;
is( $hit->{content}, $doc_2, "boosting a sub query succeeds" );
