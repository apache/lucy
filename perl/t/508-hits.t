use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 9;
use KinoSearch::Test;
use KinoSearch::Test::TestUtils qw( create_index );

my @docs     = ( 'a b', 'a a b', 'a a a b', 'x' );
my $folder   = create_index(@docs);
my $searcher = KinoSearch::Search::IndexSearcher->new( index => $folder );

my $hits = $searcher->hits(
    query      => 'a',
    offset     => 0,
    num_wanted => 1,
);
is( $hits->total_hits, 3, "total_hits" );
my $hit = $hits->next;
cmp_ok( $hit->get_score, '>', 0.0, "score field added" );
is( $hits->next, undef, "hits exhausted" );

$hits->next;
is( $hits->next, undef, "hits exhausted" );

my @retrieved;
@retrieved = ();
$hits      = $searcher->hits(
    query      => 'a',
    offset     => 0,
    num_wanted => 100,
);
is( $hits->total_hits, 3, "total_hits still correct" );
while ( my $hit = $hits->next ) {
    push @retrieved, $hit->{content};
}
is_deeply( \@retrieved, [ @docs[ 2, 1, 0 ] ], "correct content via next()" );

@retrieved = ();
$hits      = $searcher->hits(
    query      => 'a',
    offset     => 1,
    num_wanted => 100,
);
is( $hits->total_hits, 3, "total_hits correct with offset" );
while ( my $hit = $hits->next ) {
    push @retrieved, $hit->{content};
}
is( scalar @retrieved, 2, "number retrieved with offset" );
is_deeply( \@retrieved, [ @docs[ 1, 0 ] ], "correct content with offset" );
