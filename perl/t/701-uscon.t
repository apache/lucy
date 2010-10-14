use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 9;
use KinoSearch::Test::TestUtils qw( persistent_test_index_loc );
use KinoSearch::Test::USConSchema;

my $searcher = KinoSearch::Search::IndexSearcher->new(
    index => persistent_test_index_loc() );
isa_ok( $searcher, 'KinoSearch::Search::IndexSearcher' );

my %searches = (
    'United'              => 34,
    'shall'               => 50,
    'not'                 => 27,
    '"shall not"'         => 21,
    'shall not'           => 51,
    'Congress'            => 31,
    'Congress AND United' => 22,
    '(Congress AND United) OR ((Vice AND President) OR "free exercise")' =>
        28,
);

while ( my ( $qstring, $num_expected ) = each %searches ) {
    my $hits = $searcher->hits( query => $qstring );
    is( $hits->total_hits, $num_expected, $qstring );
}
