use strict;
use warnings;

use Test::More tests => 5;
use KinoSearch::Test;
use KSx::Search::MockScorer;

my @docs   = ( 1, 5, 10, 1000 );
my @scores = ( 2, 0, 0,  1 );

my $collector
    = KinoSearch::Search::Collector::SortCollector->new( wanted => 3 );
test_collect($collector);

my @got = map { $_->get_score } @{ $collector->pop_match_docs };
is_deeply( \@got, [ 2, 1, 0 ], "collect into HitQueue" );

$collector = KinoSearch::Search::Collector::SortCollector->new( wanted => 0 );
test_collect($collector);
is( $collector->get_total_hits, 4,
    "get_total_hits is accurate when no hits are requested" );
my $match_docs = $collector->pop_match_docs;
is( scalar @$match_docs, 0, "no hits wanted, so no hits returned" );

my $bit_vec = KinoSearch::Object::BitVector->new;
$collector = KinoSearch::Search::Collector::BitCollector->new(
    bit_vector => $bit_vec );
test_collect($collector);
is_deeply(
    $bit_vec->to_arrayref,
    [ 1, 5, 10, 1000 ],
    "BitCollector collects the right doc nums"
);

$bit_vec = KinoSearch::Object::BitVector->new;
my $inner_coll = KinoSearch::Search::Collector::BitCollector->new(
    bit_vector => $bit_vec );
my $offset_coll = KinoSearch::Search::Collector::OffsetCollector->new(
    collector => $inner_coll,
    offset    => 10,
);
test_collect($offset_coll);
is_deeply( $bit_vec->to_arrayref, [ 11, 15, 20, 1010 ], "Offset collector" );

sub test_collect {
    my $collector = shift;
    my $matcher   = KSx::Search::MockScorer->new(
        doc_ids => \@docs,
        scores  => \@scores,
    );
    $collector->set_matcher($matcher);
    while ( my $doc_id = $matcher->next ) {
        $collector->collect($doc_id);
    }
}
