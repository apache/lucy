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

package MultiFieldSchema;
use base qw( Lucy::Plan::Schema );
use Lucy::Analysis::StandardTokenizer;

sub new {
    my $self       = shift->SUPER::new(@_);
    my $plain_type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::StandardTokenizer->new );
    my $not_analyzed_type = Lucy::Plan::StringType->new;
    $self->spec_field( name => 'a', type => $plain_type );
    $self->spec_field( name => 'b', type => $plain_type );
    $self->spec_field( name => 'c', type => $not_analyzed_type );
    return $self;
}

package main;
use Test::More tests => 13;

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MultiFieldSchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->add_doc( { a => 'foo' } );
$indexer->add_doc( { b => 'foo' } );
$indexer->add_doc( { a => 'United States unit state' } );
$indexer->add_doc( { a => 'unit state' } );
$indexer->add_doc( { c => 'unit' } );
$indexer->commit;

my $searcher = Lucy::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits( query => 'foo' );
is( $hits->total_hits, 2, "Searcher's default is to find all fields" );

my $qparser = Lucy::Search::QueryParser->new( schema => $schema );

my $foo_leaf = Lucy::Search::LeafQuery->new( text => 'foo' );
my $multi_field_foo = Lucy::Search::ORQuery->new;
$multi_field_foo->add_child(
    Lucy::Search::TermQuery->new(
        field => $_,
        term  => 'foo'
    )
) for qw( a b c );
my $expanded = $qparser->expand($foo_leaf);
ok( $multi_field_foo->equals($expanded), "Expand LeafQuery" );

my $multi_field_bar = Lucy::Search::ORQuery->new;
$multi_field_bar->add_child(
    Lucy::Search::TermQuery->new(
        field => $_,
        term  => 'bar'
    )
) for qw( a b c );
my $not_multi_field_bar
    = Lucy::Search::NOTQuery->new( negated_query => $multi_field_bar );
my $bar_leaf = Lucy::Search::LeafQuery->new( text => 'bar' );
my $not_bar_leaf = Lucy::Search::NOTQuery->new( negated_query => $bar_leaf );
$expanded = $qparser->expand($not_bar_leaf);
ok( $not_multi_field_bar->equals($expanded), "Expand NOTQuery" );

my $query = $qparser->parse('foo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 2, "QueryParser's default is to find all fields" );

$query = $qparser->parse('b:foo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 0, "no set_heed_colons" );

$qparser->set_heed_colons(1);
$query = $qparser->parse('b:foo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 1, "set_heed_colons" );

$query = $qparser->parse('a:boffo.moffo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 0,
    "no crash for non-existent phrases under heed_colons" );

$query = $qparser->parse('a:x.nope');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 0,
    "no crash for non-existent terms under heed_colons" );

$query = $qparser->parse('nyet:x.x');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 0,
    "no crash for non-existent fields under heed_colons" );

$qparser = Lucy::Search::QueryParser->new(
    schema => $schema,
    fields => ['a'],
);
$query = $qparser->parse('foo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 1, "QueryParser fields param works" );

my $analyzer_parser = Lucy::Search::QueryParser->new(
    schema   => $schema,
    analyzer => Lucy::Analysis::EasyAnalyzer->new( language => 'en' ),
);

$hits = $searcher->hits( query => 'United States' );
is( $hits->total_hits, 1, "search finds 1 doc (prep for next text)" );

$query = $analyzer_parser->parse('unit');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 3, "QueryParser uses supplied Analyzer" );

$query = $analyzer_parser->parse('United States');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 3,
    "QueryParser uses supplied analyzer even for non-analyzed fields" );

