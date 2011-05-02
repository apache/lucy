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

package MySchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::RegexTokenizer;

sub new {
    my $self     = shift->SUPER::new(@_);
    my $analyzer = Lucy::Analysis::RegexTokenizer->new( pattern => '\S+' );
    my $type     = Lucy::Plan::FullTextType->new( analyzer => $analyzer, );
    $self->spec_field( name => 'content', type => $type );
    return $self;
}

package main;
use Test::More tests => 14;
use Lucy::Test;
use Lucy::Test::TestUtils qw( utf8_test_strings );

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

my $turd = pack( 'C*', 254, 254 );
my $polished_turd = $turd;
utf8::upgrade($polished_turd);

is( $turd, $polished_turd, "verify encoding acrobatics" );

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

$indexer->add_doc( { content => $smiley } );
$indexer->add_doc( { content => $not_a_smiley } );
$indexer->add_doc( { content => $turd } );
$indexer->commit;

my $qparser = Lucy::Search::QueryParser->new( schema => MySchema->new );
my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits( query => $qparser->parse($smiley) );
is( $hits->total_hits, 1 );
is( $hits->next->{content},
    $smiley, "Indexer and QueryParser handle UTF-8 source correctly" );

$hits = $searcher->hits( query => $qparser->parse($frowny) );
is( $hits->total_hits, 1 );
is( $hits->next->{content}, $frowny, "Indexer upgrades non-UTF-8 correctly" );

$hits = $searcher->hits( query => $qparser->parse($not_a_smiley) );
is( $hits->total_hits, 1 );
is( $hits->next->{content},
    $not_a_smiley, "QueryParser upgrades non-UTF-8 correctly" );

my $term_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => $not_a_smiley,
);
$hits = $searcher->hits( query => $term_query );
is( $hits->total_hits, 1 );
is( $hits->next->{content},
    $not_a_smiley, "TermQuery upgrades non-UTF-8 correctly" );

$term_query = Lucy::Search::TermQuery->new(
    field => 'content',
    term  => $smiley,
);

$hits = $searcher->hits( query => $term_query );
is( $hits->total_hits, 1 );
is( $hits->next->{content}, $smiley, "TermQuery handles UTF-8 correctly" );

undef $indexer;
$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->delete_by_term( field => 'content', term => $smiley );
$indexer->commit;
$searcher = Lucy::Search::IndexSearcher->new( index => $folder );

$hits = $searcher->hits( query => $smiley );
is( $hits->total_hits, 0, "delete_by_term handles UTF-8 correctly" );

$hits = $searcher->hits( query => $frowny );
is( $hits->total_hits, 1, "delete_by_term handles UTF-8 correctly" );

undef $indexer;
$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->delete_by_term( field => 'content', term => $not_a_smiley );
$indexer->commit;
$searcher = Lucy::Search::IndexSearcher->new( index => $folder );

$hits = $searcher->hits( query => $frowny );
is( $hits->total_hits, 0, "delete_by_term upgrades non-UTF-8 correctly" );
