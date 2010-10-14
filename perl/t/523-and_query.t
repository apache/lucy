use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 11;
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

my $and_query = KinoSearch::Search::ANDQuery->new;
is( $and_query->to_string, "()", "to_string (empty)" );
$and_query->add_child($a_query);
$and_query->add_child($b_query);
is( $and_query->to_string, "(content:a AND content:b)", "to_string" );

my $frozen = freeze($and_query);
my $thawed = thaw($frozen);
ok( $and_query->equals($thawed), "equals" );
$thawed->set_boost(10);
ok( !$and_query->equals($thawed), '!equals (boost)' );

my $different_children = KinoSearch::Search::ANDQuery->new(
    children => [ $a_query, $a_query ],    # a_query added twice
);
ok( !$and_query->equals($different_children),
    '!equals (different children)' );

my $one_child = KinoSearch::Search::ANDQuery->new( children => [$a_query] );
ok( !$and_query->equals($one_child), '!equals (too few children)' );

my $and_compiler = $and_query->make_compiler( searcher => $searcher );
isa_ok( $and_compiler, "KinoSearch::Search::ANDCompiler", "make_compiler" );
$frozen = freeze($and_compiler);
$thawed = thaw($frozen);
ok( $thawed->equals($and_compiler), "freeze/thaw compiler" );

my $and_scorer = $and_compiler->make_matcher(
    reader     => $reader,
    need_score => 0,
);
isa_ok( $and_scorer, "KinoSearch::Search::ANDScorer", "make_matcher" );

my $term_scorer = $one_child->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
isa_ok(
    $term_scorer,
    "KinoSearch::Search::TermScorer",
    "make_matcher compiles to child's scorer if there's only one child"
);

my $hopeless_query = KinoSearch::Search::TermQuery->new(
    field => 'nyet',
    term  => 'nein',
);
$and_query->add_child($hopeless_query);
my $nope = $and_query->make_compiler( searcher => $searcher )
    ->make_matcher( reader => $reader, need_score => 0 );
ok( !defined $nope,
    "If scorer wouldn't return any docs, make_matcher returns undef" );

