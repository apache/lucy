use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 18;
use Storable qw( freeze thaw );
use KinoSearch::Test::TestUtils qw( create_index );

my $folder = create_index( 'a', 'b', 'c c c d', 'c d', 'd' .. 'z', );
my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );
my $reader = $searcher->get_reader->get_seg_readers->[0];

my $a_query = KinoSearch::Search::TermQuery->new(
    field => 'content',
    term  => 'a'
);

my $b_query = KinoSearch::Search::TermQuery->new(
    field => 'content',
    term  => 'b'
);

for my $conjunction (qw( AND OR )) {
    my $class = "KinoSearch::Search::${conjunction}Query";
    my $polyquery = $class->new( children => [ $a_query, $b_query ] );

    my $frozen = freeze($polyquery);
    my $thawed = thaw($frozen);
    ok( $polyquery->equals($thawed), "equals" );
    $thawed->set_boost(10);
    ok( !$polyquery->equals($thawed), '!equals (boost)' );

    my $different_kids = $class->new( children => [ $a_query, $a_query ] );
    ok( !$polyquery->equals($different_kids),
        '!equals (different children)' );

    my $one_child = $class->new( children => [$a_query] );
    ok( !$polyquery->equals($one_child), '!equals (too few children)' );

    my $compiler = $polyquery->make_compiler( searcher => $searcher );
    isa_ok( $compiler, "KinoSearch::Search::${conjunction}Compiler",
        "make_compiler" );
    $frozen = freeze($compiler);
    $thawed = thaw($frozen);
    ok( $thawed->equals($compiler), "freeze/thaw compiler" );

    my $matcher
        = $compiler->make_matcher( reader => $reader, need_score => 1 );
    isa_ok(
        $matcher,
        "KinoSearch::Search::${conjunction}Scorer",
        "make_matcher with need_score"
    );

    my $term_matcher = $one_child->make_compiler( searcher => $searcher )
        ->make_matcher( reader => $reader, need_score => 0 );
    isa_ok(
        $term_matcher,
        "KinoSearch::Search::TermScorer",
        "make_matcher compiles to child's scorer if there's only one child"
    );

    my $hopeless_query = KinoSearch::Search::TermQuery->new(
        field => 'nyet',
        term  => 'nein',
    );
    my $doomed_query = KinoSearch::Search::TermQuery->new(
        field => 'luckless',
        term  => 'zero',
    );
    $polyquery
        = $class->new( children => [ $hopeless_query, $doomed_query ] );
    my $nope = $polyquery->make_compiler( searcher => $searcher )
        ->make_matcher( reader => $reader, need_score => 0 );
    ok( !defined $nope,
        "If scorer wouldn't return any docs, make_matcher returns undef" );
}

