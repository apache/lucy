use strict;
use warnings;

use Test::More tests => 11;
use KinoSearch::Test;

my $tokenizer   = KinoSearch::Analysis::Tokenizer->new;
my $other       = KinoSearch::Analysis::Tokenizer->new( pattern => '\w+' );
my $yet_another = KinoSearch::Analysis::Tokenizer->new( pattern => '\w+' );
ok( $other->equals($yet_another), "Equals" );
ok( !$tokenizer->equals($other),  "different patterns foil Equals" );

my $text = $tokenizer->split("o'malley's")->[0];
is( $text, "o'malley's", "multiple apostrophes for default pattern" );

my $inversion = KinoSearch::Analysis::Inversion->new( text => "a b c" );
$inversion = $tokenizer->transform($inversion);

my ( @token_texts, @start_offsets, @end_offsets );
while ( my $token = $inversion->next ) {
    push @token_texts,   $token->get_text;
    push @start_offsets, $token->get_start_offset;
    push @end_offsets,   $token->get_end_offset;
}
is_deeply( \@token_texts, [qw( a b c )], "correct texts" );
is_deeply( \@start_offsets, [ 0, 2, 4, ], "correctstart offsets" );
is_deeply( \@end_offsets,   [ 1, 3, 5, ], "correct end offsets" );

$tokenizer = KinoSearch::Analysis::Tokenizer->new( pattern => '.' );
$inversion = KinoSearch::Analysis::Inversion->new( text    => "a b c" );
$inversion = $tokenizer->transform($inversion);

@token_texts   = ();
@start_offsets = ();
@end_offsets   = ();
while ( my $token = $inversion->next ) {
    push @token_texts,   $token->get_text;
    push @start_offsets, $token->get_start_offset;
    push @end_offsets,   $token->get_end_offset;
}
is_deeply(
    \@token_texts,
    [ 'a', ' ', 'b', ' ', 'c' ],
    "texts: custom pattern"
);
is_deeply( \@start_offsets, [ 0 .. 4 ], "starts: custom pattern" );
is_deeply( \@end_offsets,   [ 1 .. 5 ], "ends: custom pattern" );

$inversion->reset;
$inversion   = $tokenizer->transform($inversion);
@token_texts = ();
while ( my $token = $inversion->next ) {
    push @token_texts, $token->get_text;
}
is_deeply(
    \@token_texts,
    [ 'a', ' ', 'b', ' ', 'c' ],
    "no freakout when fed multiple tokens"
);

$tokenizer = KinoSearch::Analysis::Tokenizer->new( token_re => qr/../ );
is_deeply( $tokenizer->split('aabbcc'),
    [qw( aa bb cc )], "back compat with token_re argument" );
