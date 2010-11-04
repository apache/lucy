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

use Test::More tests => 5;
use Storable qw( freeze thaw );
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a' .. 'z' );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $no_match_query = Lucy::Search::NoMatchQuery->new;
is( $no_match_query->to_string, "[NOMATCH]", "to_string" );

my $hits = $searcher->hits( query => $no_match_query );
is( $hits->total_hits, 0, "no matches" );

my $frozen = freeze($no_match_query);
my $thawed = thaw($frozen);
ok( $no_match_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$no_match_query->equals($thawed), '!equals (boost)' );

my $compiler = $no_match_query->make_compiler( searcher => $searcher );
$frozen = freeze($compiler);
$thawed = thaw($frozen);
ok( $compiler->equals($thawed), "freeze/thaw compiler" );
