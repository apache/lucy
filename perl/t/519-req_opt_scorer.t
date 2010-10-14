use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 726;
use KinoSearch::Test;
use KSx::Search::MockScorer;
use KinoSearch::Test::TestUtils qw( modulo_set doc_ids_from_td_coll );

my $sim = KinoSearch::Index::Similarity->new;

for my $req_interval ( 1 .. 10, 75 ) {
    for my $opt_interval ( 1 .. 10, 75 ) {
        check_scorer( $req_interval, $opt_interval );
        check_scorer( $opt_interval, $req_interval );
    }
}

sub check_scorer {
    my ( $req_interval, $opt_interval ) = @_;
    my $req_docs = modulo_set( $req_interval, 100 );
    my $opt_docs = modulo_set( $opt_interval, 100 );
    my $req_mock = KSx::Search::MockScorer->new(
        doc_ids => $req_docs,
        scores  => [ (1) x scalar @$req_docs ],
    );
    my $opt_mock = KSx::Search::MockScorer->new(
        doc_ids => $opt_docs,
        scores  => [ (1) x scalar @$opt_docs ],
    );
    my $req_opt_scorer = KinoSearch::Search::RequiredOptionalScorer->new(
        similarity       => $sim,
        required_matcher => $req_mock,
        optional_matcher => $opt_mock,
    );
    my $collector
        = KinoSearch::Search::Collector::SortCollector->new( wanted => 1000 );
    $req_opt_scorer->collect( collector => $collector );
    my ( $got_by_score, $got_by_id ) = doc_ids_from_td_coll($collector);
    my ( $expected_by_count, $expected_by_id )
        = calc_result_sets( $req_interval, $opt_interval );
    is( scalar @$got_by_id,
        scalar @$expected_by_id,
        "total hits: $req_interval $opt_interval"
    );

    is_deeply( $got_by_id, $expected_by_id,
        "got all docs: $req_interval $opt_interval" );

    is_deeply( $got_by_score, $expected_by_count,
        "scores accumulated: $req_interval $opt_interval" );
}

sub calc_result_sets {
    my ( $req_interval, $opt_interval ) = @_;

    my @good;
    my @better;
    for my $doc_id ( 1 .. 99 ) {
        if ( $doc_id % $req_interval == 0 ) {
            if ( $doc_id % $opt_interval == 0 ) {
                push @better, $doc_id;
            }
            else {
                push @good, $doc_id;
            }
        }
    }
    my @by_count_then_id = ( @better, @good );
    my @by_id = sort { $a <=> $b } @by_count_then_id;

    return ( \@by_count_then_id, \@by_id );
}

# Trigger destruction.
undef $sim;
