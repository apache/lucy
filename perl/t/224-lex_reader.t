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

use Test::More tests => 3;
use Lucy::Test::TestUtils qw( create_index );

my $folder = create_index(
    "What's he building in there?",
    "What's he building in there?",
    "We have a right to know."
);
my $polyreader = Lucy::Index::IndexReader->open( index => $folder );
my $seg_reader = $polyreader->get_seg_readers->[0];
my $lex_reader = $seg_reader->obtain("Lucy::Index::LexiconReader");

my $lexicon = $lex_reader->lexicon( field => 'content', term => 'building' );
isa_ok( $lexicon, 'Lucy::Index::Lexicon',
    "lexicon returns a Lucy::Index::Lexicon" );
my $tinfo = $lexicon->get_term_info;
is( $tinfo->get_doc_freq, 2, "correct place in lexicon" );

$lexicon = $lex_reader->lexicon( field => 'content' );
$lexicon->next;
is( $lexicon->get_term, 'We',
    'calling lexicon without a term returns Lexicon with iterator reset' );

