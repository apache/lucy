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
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'c c c d', 'c d', 'd' .. 'z', );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $reader = $searcher->get_reader->get_seg_readers->[0];

my $a_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'a'
);

my $b_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'b'
);

my $and_query = Lucy::Search::ANDQuery->new;
is( $and_query->to_string, "()", "to_string (empty)" );
$and_query->add_child($a_query);
$and_query->add_child($b_query);
is( $and_query->to_string, "(content:a AND content:b)", "to_string" );

my $frozen = freeze($and_query);
my $thawed = thaw($frozen);
ok( $and_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$and_query->equals($thawed), '!equals (boost)' );

my $different_children = Lucy::Search::ANDQuery->new(
    children => [ $a_query, $a_query ],    # a_query added twice
);
ok( !$and_query->equals($different_children),
    '!equals (different children)' );

my $one_child = Lucy::Search::ANDQuery->new( children => [$a_query] );
ok( !$and_query->equals($one_child), '!equals (too few children)' );

my $and_compiler = $and_query->make_compiler( searcher => $searcher );
isa_ok( $and_compiler, "Lucy::Search::ANDCompiler", "make_compiler" );
$frozen = freeze($and_compiler);
$thawed = thaw($frozen);
ok( $thawed->equals($and_compiler), "freeze/thaw compiler" );

my $and_matcher = $and_compiler->make_matcher(
    reader     => $reader,
    need_score => 0,
);
isa_ok( $and_matcher, "Lucy::Search::ANDMatcher", "make_matcher" );

my $term_matcher = $one_child->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
isa_ok( $term_matcher, "Lucy::Search::TermMatcher",
    "make_matcher compiles to child's Matcher if there's only one child" );

my $hopeless_query = Lucy::Search::TermQuery->new(
    field => 'nyet',
    term  => 'nein',
);
$and_query->add_child($hopeless_query);
my $nope = $and_query->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
ok( !defined $nope,
    "If matcher wouldn't return any docs, make_matcher returns undef" );

