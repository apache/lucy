use strict;
use warnings;

use Test::More tests => 11;
use Clownfish::Type::Float;
use Clownfish::Parser;

my $float_type = Clownfish::Type::Float->new(
    specifier => 'float',
    const     => 1,
);
ok( $float_type->const, "const" );
is( $float_type->get_specifier, "float" );
like( $float_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::Parser->new;

for my $specifier (qw( float double)) {
    is( $parser->c_float_specifier($specifier),
        $specifier, "c_float_specifier: $specifier" );
    isa_ok( $parser->float_type($specifier), "Clownfish::Type::Float" );
    isa_ok( $parser->float_type("const $specifier"),
        "Clownfish::Type::Float" );
    my $bogus = $specifier . "y";
    ok( !$parser->c_float_specifier($bogus),
        "c_float_specifier guards against partial word matches" );
}

