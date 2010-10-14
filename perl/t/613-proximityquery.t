use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 11;
use Storable qw( freeze thaw );
use KinoSearch::Test;
use KinoSearch::Test::TestUtils qw( create_index );
use KSx::Search::ProximityQuery;

# this is better than 'x a b c d a b c d' because its
# posting weight is higher, presumably because
# it is a shorter doc (higher density?)
my $best_match = 'a b c d x x a';

my @docs = (
    1 .. 20,
    'a b c a b c a b c d',
    'x a b c d a b c d',
    'a c b d', 'a x x x b x x x c x x x x x x d x',
    $best_match, 'a' .. 'z',
);

my $folder = create_index(@docs);
my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [],
    within => 10,
);
is( $proximity_query->to_string, 'content:""~10',
    "empty ProximityQuery to_string" );
$proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( d a )],
    within => 10,
);
is( $proximity_query->to_string, 'content:"d a"~10', "to_string" );

my $hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 2, "correct number of hits" );
my $first_hit = $hits->next;
is( $first_hit->{content}, $best_match, 'best match appears first' );

my $second_hit = $hits->next;
ok( $first_hit->get_score > $second_hit->get_score,
    "best match scores higher: "
        . $first_hit->get_score . " > "
        . $second_hit->get_score
);

$proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( c a )],
    within => 10,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 3, 'avoid underflow when subtracting offset' );

$proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( b d )],
    within => 10,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 4, 'offset starting from zero' );

my $frozen = freeze($proximity_query);
my $thawed = thaw($frozen);
$hits = $searcher->hits( query => $thawed );
is( $hits->total_hits, 4, 'freeze/thaw' );

my $proximity_compiler
    = $proximity_query->make_compiler( searcher => $searcher, );
$frozen = freeze($proximity_compiler);
$thawed = thaw($frozen);
ok( $proximity_compiler->equals($thawed), "freeze/thaw compiler" );

$proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( x d )],
    within => 4,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 2, 'within range is exclusive' );
$proximity_query = KSx::Search::ProximityQuery->new(
    field  => 'content',
    terms  => [qw( x d )],
    within => 3,
);
$hits = $searcher->hits( query => $proximity_query );
is( $hits->total_hits, 1, 'within range is exclusive' );
