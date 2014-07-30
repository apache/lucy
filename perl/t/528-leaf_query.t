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
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'b c', 'c', 'c d', 'd', 'e' );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );
my $reader = $searcher->get_reader;

my $leaf_query = Lucy::Search::LeafQuery->new(
    field => 'content',
    text  => 'b'
);
my $no_field_leaf_query = Lucy::Search::LeafQuery->new( text => 'b' );
is( $leaf_query->to_string,          "content:b", "to_string" );
is( $no_field_leaf_query->to_string, "b",         "no field to_string" );

is( $leaf_query->get_field, "content", "get field" );
ok( !defined $no_field_leaf_query->get_field, "get null field" );
is( $leaf_query->get_text, 'b', 'get text' );

ok( !$leaf_query->equals($no_field_leaf_query), "!equals (field/nofield)" );
ok( !$no_field_leaf_query->equals($leaf_query), "!equals (nofield/field)" );

my $diff_field = Lucy::Search::LeafQuery->new( field => 'oink', text => 'b' );
ok( !$diff_field->equals($leaf_query), "!equals (different field)" );

my $diff_text
    = Lucy::Search::LeafQuery->new( field => 'content', text => 'c' );
ok( !$diff_text->equals($leaf_query), "!equals (different text)" );

eval {
    $leaf_query->make_compiler(
        searcher => $searcher,
        boost    => $leaf_query->get_boost,
    );
};
like( $@, qr/Make_Compiler/, "Make_Compiler throws error" );

my $frozen = freeze($leaf_query);
my $thawed = thaw($frozen);
ok( $leaf_query->equals($thawed), "freeze/thaw and equals" );
$leaf_query->set_boost(2);
ok( !$leaf_query->equals($thawed), "!equals (boost)" );

