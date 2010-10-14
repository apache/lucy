use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 9;
use Storable qw( freeze thaw );
use KinoSearch::Test;
use KinoSearch::Test::TestUtils qw( create_index );

my $best_match = 'x a b c d a b c d';

my @docs = (
    1 .. 20,
    'a b c a b c a b c d',
    'a b c d x x a',
    'a c b d', 'a x x x b x x x c x x x x x x d x',
    $best_match, 'a' .. 'z',
);

my $folder = create_index(@docs);
my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $phrase_query = KinoSearch::Search::PhraseQuery->new(
    field => 'content',
    terms => [],
);
is( $phrase_query->to_string, 'content:""', "empty PhraseQuery to_string" );
$phrase_query = KinoSearch::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( a b c d )],
);
is( $phrase_query->to_string, 'content:"a b c d"', "to_string" );

my $hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 3, "correct number of hits" );
my $first_hit = $hits->next;
is( $first_hit->{content}, $best_match, 'best match appears first' );

my $second_hit = $hits->next;
ok( $first_hit->get_score > $second_hit->get_score,
    "best match scores higher: "
        . $first_hit->get_score . " > "
        . $second_hit->get_score
);

$phrase_query = KinoSearch::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( c a )],
);
$hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 1, 'avoid underflow when subtracting offset' );

# "a b c"
$phrase_query = KinoSearch::Search::PhraseQuery->new(
    field => 'content',
    terms => [qw( a b c )],
);
$hits = $searcher->hits( query => $phrase_query );
is( $hits->total_hits, 3, 'offset starting from zero' );

my $frozen = freeze($phrase_query);
my $thawed = thaw($frozen);
$hits = $searcher->hits( query => $thawed );
is( $hits->total_hits, 3, 'freeze/thaw' );

my $phrase_compiler = $phrase_query->make_compiler( searcher => $searcher );
$frozen = freeze($phrase_compiler);
$thawed = thaw($frozen);
ok( $phrase_compiler->equals($thawed), "freeze/thaw compiler" );
