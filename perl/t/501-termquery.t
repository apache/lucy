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

use Test::More tests => 12;
use Storable qw( freeze thaw );
use Lucy::Test;
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'c c c d', 'c d', 'd' .. 'z', );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $term_query
    = Lucy::Search::TermQuery->new( field => 'content', term => 'c' );
is( $term_query->to_string, "content:c", "to_string" );

my $hits = $searcher->hits( query => $term_query );
is( $hits->total_hits, 2, "correct number of hits returned" );

my $hit = $hits->next;
is( $hit->{content}, 'c c c d', "most relevant doc is highest" );

$hit = $hits->next;
is( $hit->{content}, 'c d', "second most relevant" );

my $frozen = freeze($term_query);
my $thawed = thaw($frozen);
is( $thawed->get_field, 'content', "field survives freeze/thaw" );
is( $thawed->get_term,  'c',       "term survives freeze/thaw" );
is( $thawed->get_boost, $term_query->get_boost,
    "boost survives freeze/thaw" );
ok( $thawed->equals($term_query), "equals" );
$thawed->set_boost(10);
ok( !$thawed->equals($term_query), "!equals (boost)" );
my $different_term = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'd'
);
my $different_field = Lucy::Search::TermQuery->new(
    field => 'title',
    term  => 'c'
);
ok( !$term_query->equals($different_term),  "!equals (term)" );
ok( !$term_query->equals($different_field), "!equals (field)" );

my $term_compiler = $term_query->make_compiler( searcher => $searcher );
$frozen = freeze($term_compiler);
$thawed = thaw($frozen);
ok( $term_compiler->equals($thawed), "freeze/thaw compiler" );
