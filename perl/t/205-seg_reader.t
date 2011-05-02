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
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index(
    "What's he building in there?",
    "What's he building in there?",
    "We have a right to know."
);
my $polyreader = Lucy::Index::IndexReader->open( index => $folder );
my $reader = $polyreader->get_seg_readers->[0];

isa_ok( $reader, 'Lucy::Index::SegReader' );

is( $reader->doc_max, 3, "doc_max returns correct number" );

my $lex_reader = $reader->fetch("Lucy::Index::LexiconReader");
isa_ok( $lex_reader, 'Lucy::Index::LexiconReader', "fetch() a component" );
ok( !defined( $reader->fetch("nope") ),
    "fetch() returns undef when component can't be found" );
$lex_reader = $reader->obtain("Lucy::Index::LexiconReader");
isa_ok( $lex_reader, 'Lucy::Index::LexiconReader', "obtain() a component" );
eval { $reader->obtain("boom."); };
like( $@, qr/boom/, "obtain blows up when component can't be found" );

is_deeply( $reader->seg_readers, [$reader], "seg_readers" );
is_deeply( $reader->offsets,     [0],       "offsets" );

