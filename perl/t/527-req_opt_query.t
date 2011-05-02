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

use Test::More tests => 7;
use Storable qw( freeze thaw );
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'b c', 'c', 'c d', 'd', 'e' );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $reader = $searcher->get_reader->get_seg_readers->[0];

my $b_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'b'
);
my $c_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'c'
);
my $x_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => 'x'
);

my $req_opt_query = Lucy::Search::RequiredOptionalQuery->new(
    required_query => $b_query,
    optional_query => $c_query,
);
is( $req_opt_query->to_string, "(+content:b content:c)", "to_string" );

my $compiler = $req_opt_query->make_compiler( searcher => $searcher );
my $frozen   = freeze($compiler);
my $thawed   = thaw($frozen);
ok( $thawed->equals($compiler), "freeze/thaw compiler" );
my $matcher = $compiler->make_matcher( reader => $reader, need_score => 1 );
isa_ok( $matcher, 'Lucy::Search::RequiredOptionalMatcher' );

$req_opt_query = Lucy::Search::RequiredOptionalQuery->new(
    required_query => $b_query,
    optional_query => $x_query,
);
$matcher = $req_opt_query->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
isa_ok( $matcher, 'Lucy::Search::TermMatcher',
    "return required matcher only when opt matcher doesn't match" );

$req_opt_query = Lucy::Search::RequiredOptionalQuery->new(
    required_query => $x_query,
    optional_query => $b_query,
);
$matcher = $req_opt_query->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
ok( !defined($matcher), "if required matcher has no match, return undef" );

$frozen = freeze($req_opt_query);
$thawed = thaw($frozen);
ok( $req_opt_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$req_opt_query->equals($thawed), '!equals (boost)' );

