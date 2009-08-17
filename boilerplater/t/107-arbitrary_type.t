use strict;
use warnings;

use Test::More tests => 12;
use Boilerplater::Type::Arbitrary;
use Boilerplater::Parser;

my $foo_type = Boilerplater::Type::Arbitrary->new(
    parcel    => 'Boil',
    specifier => "foo_t",
);
is( $foo_type->get_specifier, "foo_t", "get_specifier" );
is( $foo_type->to_c,          "foo_t", "to_c" );

my $compare_t_type = Boilerplater::Type::Arbitrary->new(
    parcel    => 'Boil',
    specifier => "Sort_compare_t",
);
is( $compare_t_type->get_specifier,
    "boil_Sort_compare_t", "Prepend prefix to specifier" );
is( $compare_t_type->to_c, "boil_Sort_compare_t", "to_c" );

my $evil_twin = Boilerplater::Type::Arbitrary->new(
    parcel    => 'Boil',
    specifier => "foo_t",
);
ok( $foo_type->equals($evil_twin), "equals" );
ok( !$foo_type->equals($compare_t_type),
    "equals spoiled by different specifier"
);

my $parser = Boilerplater::Parser->new;

for my $specifier (qw( foo_t Sort_compare_t )) {
    is( $parser->arbitrary_type_specifier($specifier),
        $specifier, 'arbitrary_type_specifier' );
    isa_ok(
        $parser->arbitrary_type($specifier),
        "Boilerplater::Type::Arbitrary"
    );
    ok( !$parser->arbitrary_type_specifier( $specifier . "_y_p_e" ),
        "arbitrary_type_specifier guards against partial word matches"
    );
}

