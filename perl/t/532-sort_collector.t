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

use Test::More tests => 32;
use Lucy::Test;
use List::Util qw( shuffle );
use LucyX::Search::MockMatcher;

my $schema = Lucy::Plan::Schema->new;
my $type = Lucy::Plan::StringType->new( sortable => 1 );
$schema->spec_field( name => 'letter', type => $type );
$schema->spec_field( name => 'number', type => $type );
$schema->spec_field( name => 'id',     type => $type );

my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

my @letters = 'a' .. 'z';
my @numbers = 1 .. 5;
my @docs    = (
    { letter => 'c', number => '4', id => 1, },
    { letter => 'b', number => '2', id => 2, },
    { letter => 'a', number => '5', id => 3, },
);
for my $id ( 4 .. 100 ) {
    my $doc = {
        letter => $letters[ rand @letters ],
        number => $numbers[ rand @numbers ],
        id     => $id,
    };
    push @docs, $doc;
}
$indexer->add_doc($_) for @docs;
$indexer->commit;

my $polyreader = Lucy::Index::IndexReader->open( index => $folder );
my $seg_reader = $polyreader->get_seg_readers->[0];

my $by_letter = Lucy::Search::SortSpec->new(
    rules => [
        Lucy::Search::SortRule->new( field => 'letter' ),
        Lucy::Search::SortRule->new( type  => 'doc_id' ),
    ]
);

my $collector = Lucy::Search::Collector::SortCollector->new(
    sort_spec => $by_letter,
    schema    => $schema,
    wanted    => 1,
);

$collector->set_reader($seg_reader);
$collector->collect($_) for 1 .. 100;
my $match_docs = $collector->pop_match_docs;
is( $match_docs->[0]->get_doc_id,
    3, "Early doc numbers preferred by collector" );

my @docs_and_scores;
my %uniq_doc_ids;
for ( 1 .. 30 ) {
    my $doc_num = int( rand(10000) ) + 1;
    while ( $uniq_doc_ids{$doc_num} ) {
        $doc_num = int( rand(10000) ) + 1;
    }
    $uniq_doc_ids{$doc_num} = 1;
    push @docs_and_scores, [ $doc_num, rand(10) ];
}
@docs_and_scores = sort { $a->[0] <=> $b->[0] } @docs_and_scores;
my @ranked
    = sort { $b->[1] <=> $a->[1] || $a->[1] <=> $b->[1] } @docs_and_scores;
my @doc_ids = map { $_->[0] } @docs_and_scores;
my @scores  = map { $_->[1] } @docs_and_scores;

for my $size ( 0 .. @doc_ids ) {
    my $matcher = LucyX::Search::MockMatcher->new(
        doc_ids => \@doc_ids,
        scores  => \@scores,
    );
    my $collector
        = Lucy::Search::Collector::SortCollector->new( wanted => $size, );
    $collector->set_matcher($matcher);
    $matcher->collect( collector => $collector );

    my @wanted;
    if ($size) {
        @wanted = map { $_->[0] } @ranked[ 0 .. $size - 1 ];
    }
    else {
        @wanted = ();
    }
    my @got = map { $_->get_doc_id } @{ $collector->pop_match_docs };
    is_deeply( \@got, \@wanted, "random docs and scores, wanted = $size" );
}
