use strict;
use warnings;
use lib 'buildlib';

use Test::More tests => 4;
use KinoSearch::Test::TestUtils qw( utf8_test_strings );

my $inversion = KinoSearch::Analysis::Inversion->new;
$inversion->append(
    KinoSearch::Analysis::Token->new(
        text         => "car",
        start_offset => 0,
        end_offset   => 3,
    ),
);
$inversion->append(
    KinoSearch::Analysis::Token->new(
        text         => "bike",
        start_offset => 10,
        end_offset   => 14,
    ),
);
$inversion->append(
    KinoSearch::Analysis::Token->new(
        text         => "truck",
        start_offset => 20,
        end_offset   => 25,
    ),
);

my @texts;
while ( my $token = $inversion->next ) {
    push @texts, $token->get_text;
}
is_deeply( \@texts, [qw( car bike truck )], "return tokens in order" );

$inversion = KinoSearch::Analysis::Inversion->new;
$inversion->append(
    KinoSearch::Analysis::Token->new(
        text         => "foo",
        start_offset => 0,
        end_offset   => 3,
        pos_inc      => 10,
    ),
);
$inversion->append(
    KinoSearch::Analysis::Token->new(
        text         => "bar",
        start_offset => 4,
        end_offset   => 7,
        pos_inc      => ( 2**31 - 2 ),
    ),
);
eval { $inversion->invert; };
like( $@, qr/position/, "catch overflow in token position calculation" );

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

$inversion = KinoSearch::Analysis::Inversion->new( text => $smiley );
is( $inversion->next->get_text,
    $smiley, "Inversion->new handles UTF-8 correctly" );
$inversion = KinoSearch::Analysis::Inversion->new( text => $not_a_smiley );
is( $inversion->next->get_text,
    $frowny, "Inversion->new upgrades non-UTF-8 correctly" );
