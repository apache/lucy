use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 6;

use KinoSearch::Test;
use KinoSearch::Search::Span;

my $span = KinoSearch::Search::Span->new(
    offset => 2,
    length => 3,
    weight => 7,
);

is( $span->get_offset, 2, "get_offset" );
is( $span->get_length, 3, "get_length" );
is( $span->get_weight, 7, "get_weight" );

$span->set_offset(10);
$span->set_length(1);
$span->set_weight(4);

is( $span->get_offset, 10, "set_offset" );
is( $span->get_length, 1,  "set_length" );
is( $span->get_weight, 4,  "set_weight" );

