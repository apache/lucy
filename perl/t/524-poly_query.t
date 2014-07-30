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

use Test::More tests => 18;
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

for my $conjunction (qw( AND OR )) {
    my $class = "Lucy::Search::${conjunction}Query";
    my $polyquery = $class->new( children => [ $a_query, $b_query ] );

    my $frozen = freeze($polyquery);
    my $thawed = thaw($frozen);
    ok( $polyquery->equals($thawed), "equals" );
    $thawed->set_boost(10);
    ok( !$polyquery->equals($thawed), '!equals (boost)' );

    my $different_kids = $class->new( children => [ $a_query, $a_query ] );
    ok( !$polyquery->equals($different_kids),
        '!equals (different children)' );

    my $one_child = $class->new( children => [$a_query] );
    ok( !$polyquery->equals($one_child), '!equals (too few children)' );

    my $compiler = $polyquery->make_compiler(
        searcher => $searcher,
        boost    => $polyquery->get_boost,
    );
    isa_ok( $compiler, "Lucy::Search::${conjunction}Compiler",
        "make_compiler" );
    $frozen = freeze($compiler);
    $thawed = thaw($frozen);
    ok( $thawed->equals($compiler), "freeze/thaw compiler" );

    my $matcher
        = $compiler->make_matcher( reader => $reader, need_score => 1 );
    my $wanted_class
        = $conjunction eq 'AND'
        ? 'Lucy::Search::ANDMatcher'
        : 'Lucy::Search::ORScorer';
    isa_ok( $matcher, $wanted_class, "make_matcher with need_score" );

    my $term_matcher = $one_child->make_compiler(
        searcher => $searcher,
        boost    => $one_child->get_boost,
    )->make_matcher( reader => $reader, need_score => 0 );
    isa_ok( $term_matcher, "Lucy::Search::TermMatcher",
        "make_matcher compiles to child's Matcher if there's only one child"
    );

    my $hopeless_query = Lucy::Search::TermQuery->new(
        field => 'nyet',
        term  => 'nein',
    );
    my $doomed_query = Lucy::Search::TermQuery->new(
        field => 'luckless',
        term  => 'zero',
    );
    $polyquery
        = $class->new( children => [ $hopeless_query, $doomed_query ] );
    my $nope = $polyquery->make_compiler(
        searcher => $searcher,
        boost    => $polyquery->get_boost,
    )->make_matcher( reader => $reader, need_score => 0 );
    ok( !defined $nope,
        "If Matcher wouldn't return any docs, make_matcher returns undef" );
}

