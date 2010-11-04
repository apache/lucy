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
use Lucy::Test;

my $match_doc = Lucy::Search::MatchDoc->new(
    doc_id => 31,
    score  => 5.0,
);
is( $match_doc->get_doc_id, 31,    "get_doc_id" );
is( $match_doc->get_score,  5.0,   "get_score" );
is( $match_doc->get_values, undef, "get_values" );
my $match_doc_copy = thaw( freeze($match_doc) );
is( $match_doc_copy->get_doc_id, $match_doc->get_doc_id,
    "doc_id survives serialization" );
is( $match_doc_copy->get_score, $match_doc->get_score,
    "score survives serialization" );
is( $match_doc_copy->get_values, $match_doc->get_values,
    "empty values still empty after serialization" );

my $values = Lucy::Object::VArray->new( capacity => 4 );
$values->store( 0, Lucy::Object::CharBuf->new("foo") );
$values->store( 3, Lucy::Object::CharBuf->new("bar") );
$match_doc = Lucy::Search::MatchDoc->new(
    doc_id => 120,
    score  => 35,
    values => $values,
);
$match_doc_copy = thaw( freeze($match_doc) );
is_deeply(
    $match_doc_copy->get_values,
    [ 'foo', undef, undef, 'bar' ],
    "values array survives serialization"
);

