use strict;
use warnings;

use Test::More tests => 49;
use Boilerplater::Type::Object;
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;

# Set and leave parcel.
my $parcel = $parser->parcel_definition('parcel Boil;')
    or die "failed to process parcel_definition";

for my $bad_specifier (qw( foo fooBar Foo_Bar FOOBAR 1Foo 1FOO )) {
    ok( !$parser->object_type_specifier($bad_specifier),
        "reject bad object_type_specifier $bad_specifier"
    );
    eval {
        my $type = Boilerplater::Type::Object->new(
            parcel    => 'Boil',
            specifier => $bad_specifier,
        );
    };
    like( $@, qr/specifier/,
        "constructor rejects bad specifier $bad_specifier" );
}

for my $specifier (qw( Foo FooJr FooIII Foo4th )) {
    is( $parser->object_type_specifier($specifier),
        $specifier, "object_type_specifier: $specifier" );
    isa_ok( $parser->object_type("$specifier*"),
        "Boilerplater::Type::Object", "$specifier*" );
    isa_ok( $parser->object_type("const $specifier*"),
        "Boilerplater::Type::Object", "const $specifier*" );
    isa_ok( $parser->object_type("incremented $specifier*"),
        "Boilerplater::Type::Object", "incremented $specifier*" );
    isa_ok( $parser->object_type("decremented $specifier*"),
        "Boilerplater::Type::Object", "decremented $specifier*" );
}

eval { my $type = Boilerplater::Type::Object->new };
like( $@, qr/specifier/i, "specifier required" );

for ( 0, 2 ) {
    eval {
        my $type = Boilerplater::Type::Object->new(
            specifier   => 'Foo',
            indirection => $_,
        );
    };
    like( $@, qr/indirection/i, "invalid indirection of $_" );
}

my $foo_type    = Boilerplater::Type::Object->new( specifier => 'Foo' );
my $another_foo = Boilerplater::Type::Object->new( specifier => 'Foo' );
ok( $foo_type->equals($another_foo), "equals" );

my $bar_type = Boilerplater::Type::Object->new( specifier => 'Bar' );
ok( !$foo_type->equals($bar_type), "different specifier spoils equals" );

my $foreign_foo = Boilerplater::Type::Object->new(
    specifier => 'Foo',
    parcel    => 'Foreign',
);
ok( !$foo_type->equals($foreign_foo), "different parcel spoils equals" );
is( $foreign_foo->get_specifier, "foreign_Foo",
    "prepend parcel prefix to specifier" );

my $incremented_foo = Boilerplater::Type::Object->new(
    specifier   => 'Foo',
    incremented => 1,
);
ok( $incremented_foo->incremented, "incremented" );
ok( !$foo_type->incremented,       "not incremented" );
ok( !$foo_type->equals($incremented_foo),
    "different incremented spoils equals"
);

my $decremented_foo = Boilerplater::Type::Object->new(
    specifier   => 'Foo',
    decremented => 1,
);
ok( $decremented_foo->decremented, "decremented" );
ok( !$foo_type->decremented,       "not decremented" );
ok( !$foo_type->equals($decremented_foo),
    "different decremented spoils equals"
);

my $const_foo = Boilerplater::Type::Object->new(
    specifier => 'Foo',
    const     => 1,
);
ok( !$foo_type->equals($const_foo), "different const spoils equals" );
like( $const_foo->to_c, qr/const/, "const included in C representation" );

my $string_type = Boilerplater::Type::Object->new( specifier => 'CharBuf', );
ok( !$foo_type->is_string_type,   "Not is_string_type" );
ok( $string_type->is_string_type, "is_string_type" );

