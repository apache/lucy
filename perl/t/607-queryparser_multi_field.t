use strict;
use warnings;
use lib 'buildlib';

package MultiFieldSchema;
use base qw( KinoSearch::Plan::Schema );
use KinoSearch::Analysis::Tokenizer;

sub new {
    my $self       = shift->SUPER::new(@_);
    my $plain_type = KinoSearch::Plan::FullTextType->new(
        analyzer => KinoSearch::Analysis::Tokenizer->new );
    my $not_analyzed_type = KinoSearch::Plan::StringType->new;
    $self->spec_field( name => 'a', type => $plain_type );
    $self->spec_field( name => 'b', type => $plain_type );
    $self->spec_field( name => 'c', type => $not_analyzed_type );
    return $self;
}

package main;
use Test::More tests => 13;

my $folder  = KinoSearch::Store::RAMFolder->new;
my $schema  = MultiFieldSchema->new;
my $indexer = KinoSearch::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->add_doc( { a => 'foo' } );
$indexer->add_doc( { b => 'foo' } );
$indexer->add_doc( { a => 'United States unit state' } );
$indexer->add_doc( { a => 'unit state' } );
$indexer->add_doc( { c => 'unit' } );
$indexer->commit;

my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits( query => 'foo' );
is( $hits->total_hits, 2, "Searcher's default is to find all fields" );

my $qparser = KinoSearch::Search::QueryParser->new( schema => $schema );

my $foo_leaf = KinoSearch::Search::LeafQuery->new( text => 'foo' );
my $multi_field_foo = KinoSearch::Search::ORQuery->new;
$multi_field_foo->add_child(
    KinoSearch::Search::TermQuery->new(
        field => $_,
        term  => 'foo'
    )
) for qw( a b c );
my $expanded = $qparser->expand($foo_leaf);
ok( $multi_field_foo->equals($expanded), "Expand LeafQuery" );

my $multi_field_bar = KinoSearch::Search::ORQuery->new;
$multi_field_bar->add_child(
    KinoSearch::Search::TermQuery->new(
        field => $_,
        term  => 'bar'
    )
) for qw( a b c );
my $not_multi_field_bar
    = KinoSearch::Search::NOTQuery->new( negated_query => $multi_field_bar );
my $bar_leaf = KinoSearch::Search::LeafQuery->new( text => 'bar' );
my $not_bar_leaf
    = KinoSearch::Search::NOTQuery->new( negated_query => $bar_leaf );
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

$qparser = KinoSearch::Search::QueryParser->new(
    schema => $schema,
    fields => ['a'],
);
$query = $qparser->parse('foo');
$hits = $searcher->hits( query => $query );
is( $hits->total_hits, 1, "QueryParser fields param works" );

my $analyzer_parser = KinoSearch::Search::QueryParser->new(
    schema   => $schema,
    analyzer => KinoSearch::Analysis::PolyAnalyzer->new( language => 'en' ),
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

