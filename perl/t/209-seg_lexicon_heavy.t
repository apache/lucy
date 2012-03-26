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

my @docs;
my @chars = ( 'a' .. 'z', 'B' .. 'E', 'G' .. 'Z' );
for ( 0 .. 1000 ) {
    my $content = '';
    for my $num_words ( 0 .. int( rand(20) ) ) {
        for my $num_chars ( 1 .. int( rand(10) ) ) {
            $content .= @chars[ rand(@chars) ];
        }
        $content .= ' ';
    }
    push @docs, "$content\n";
}
my $folder = create_index(
    ( 1 .. 1000 ),
    ( ("a") x 100 ),
    "Foo",
    @docs,
    "Foo",
    "A MAN",
    "A PLAN",
    "A CANAL",
    "PANAMA"
);
my $schema = Lucy::Test::TestSchema->new;

my $snapshot = Lucy::Index::Snapshot->new->read_file( folder => $folder );
my $segment = Lucy::Index::Segment->new( number => 1 );
$segment->read_file($folder);
my $lex_reader = Lucy::Index::DefaultLexiconReader->new(
    schema   => $schema,
    folder   => $folder,
    snapshot => $snapshot,
    segments => [$segment],
    seg_tick => 0,
);

my $lexicon = $lex_reader->lexicon( field => 'content' );
$lexicon->next;
my $last_text = $lexicon->get_term;
$lexicon->next;
my $current_text;
my $num_iters = 2;
while (1) {
    $current_text = $lexicon->get_term;
    last unless $current_text gt $last_text;
    last unless $lexicon->next;
    $num_iters++;
    $current_text = $last_text;
}
cmp_ok( $last_text, 'lt', $current_text, "term texts in sorted order" );

$lexicon->seek('A');
my $tinfo = $lexicon->get_term_info();
is( $tinfo->get_doc_freq, 3, "correct retrieval #1" );

$lexicon->seek('Foo');
$tinfo = $lexicon->get_term_info();
is( $tinfo->get_doc_freq, 2, "correct retrieval #2" );
