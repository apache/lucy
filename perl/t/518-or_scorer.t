use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 900;
use KinoSearch::Test;
use KSx::Search::MockScorer;
use KinoSearch::Test::TestUtils qw( modulo_set doc_ids_from_td_coll );

my $sim = KinoSearch::Index::Similarity->new;

for my $interval_a ( 1 .. 10 ) {
    for my $interval_b ( 5 .. 10 ) {
        check_scorer( $interval_a, $interval_b );
        for my $interval_c ( 30, 75 ) {
            check_scorer( $interval_a, $interval_b, $interval_c );
            check_scorer( $interval_c, $interval_b, $interval_a );
        }
    }
}

sub check_scorer {
    my @intervals = @_;
    my @doc_id_arrays = map { modulo_set( $_, 100 ) } @intervals;
    my $subscorers
        = KinoSearch::Object::VArray->new( capacity => scalar @intervals );
    for my $doc_id_array (@doc_id_arrays) {
        my $mock = KSx::Search::MockScorer->new(
            doc_ids => $doc_id_array,
            scores  => [ (1) x scalar @$doc_id_array ],
        );
        $subscorers->push($mock);
    }

    my $or_scorer = KinoSearch::Search::ORScorer->new(
        similarity => $sim,
        children   => $subscorers,
    );
    my $collector
        = KinoSearch::Search::Collector::SortCollector->new( wanted => 100 );
    $or_scorer->collect( collector => $collector );
    my ( $got_by_score, $got_by_id ) = doc_ids_from_td_coll($collector);
    my ( $expected_by_count, $expected_by_id )
        = union_doc_id_sets(@doc_id_arrays);
    is( scalar @$got_by_id,
        scalar @$expected_by_id,
        "total hits: @intervals"
    );
    is_deeply( $got_by_id, $expected_by_id, "got all docs: @intervals" );
    is_deeply( $got_by_score, $expected_by_count,
        "scores accumulated: @intervals" );
}

sub union_doc_id_sets {
    my @arrays = @_;
    my %scores;
    for my $array (@arrays) {
        $scores{$_} += 1 for @$array;
    }
    my @by_count_then_id = sort { $scores{$b} <=> $scores{$a} or $a <=> $b }
        keys %scores;
    my @by_id = sort { $a <=> $b } keys %scores;
    return ( \@by_count_then_id, \@by_id );
}

# Trigger destruction.
undef $sim;
