use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 1;
use KinoSearch::Test::TestUtils qw( create_index );

my $good   = "x a x x x x b x x x x c x";
my $better = "x x x x a x b x c x x x x";
my $best   = "x x x x x a b c x x x x x";
my $folder = create_index( $good, $better, $best );

my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits( query => 'a b c' );

my @contents;
while ( my $hit = $hits->next ) {
    push @contents, $hit->{content};
}

TODO: {
    local $TODO = "positions not passed to boolscorer correctly yet";
    is_deeply(
        \@contents,
        [ $best, $better, $good ],
        "proximity helps boost scores"
    );
}

