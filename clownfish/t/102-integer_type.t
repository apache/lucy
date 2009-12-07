use strict;
use warnings;

use Test::More tests => 60;
use Clownfish::Type::Integer;
use Clownfish::Parser;

my $integer_type = Clownfish::Type::Integer->new(
    specifier => 'i32_t',
    const     => 1,
);
ok( $integer_type->const, "const" );
is( $integer_type->get_specifier, "i32_t" );
like( $integer_type->to_c, qr/chy_i32_t/,
    "prepend 'chy_' for C representation" );
like( $integer_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::Parser->new;

my @chy_specifiers = qw(
    bool_t
    i8_t
    i16_t
    i32_t
    i64_t
    u8_t
    u16_t
    u32_t
    u64_t
);
my @c_specifiers = qw(
    char
    short
    int
    long
    size_t
);

for my $chy_specifier (@chy_specifiers) {
    is( $parser->chy_integer_specifier($chy_specifier),
        $chy_specifier, "chy_integer_specifier: $chy_specifier" );
    isa_ok( $parser->chy_integer_type($chy_specifier),
        "Clownfish::Type::Integer" );
    isa_ok( $parser->chy_integer_type("const $chy_specifier"),
        "Clownfish::Type::Integer" );
    my $bogus = $chy_specifier . "oot_toot";
    ok( !$parser->chy_integer_specifier($bogus),
        "chy_integer_specifier guards against partial word matches" );
}

for my $c_specifier (@c_specifiers) {
    is( $parser->c_integer_specifier($c_specifier),
        $c_specifier, "c_integer_specifier: $c_specifier" );
    isa_ok(
        $parser->c_integer_type($c_specifier),
        "Clownfish::Type::Integer"
    );
    isa_ok( $parser->c_integer_type("const $c_specifier"),
        "Clownfish::Type::Integer" );
    my $bogus = $c_specifier . "y";
    ok( !$parser->c_integer_specifier($bogus),
        "c_integer_specifier guards against partial word matches" );
}

