use strict;
use warnings;

use Test::More tests => 24;
use Clownfish::Type::Object;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;

is( $parser->type_postfix($_), $_, "postfix: $_" )
    for ( '[]', '[A_CONSTANT]', '*' );
is( $parser->type_postfix('[ FOO ]'), '[FOO]', "type_postfix: [ FOO ]" );

my @composite_type_strings = (
    qw(
        char*
        char**
        char***
        int32_t*
        Obj**
        neato_method_t[]
        neato_method_t[1]
        multi_dimensional_t[SOME][A_FEW]
        ),
    'char * * ',
    'const Obj**',
    'const void*',
);

for my $input (@composite_type_strings) {
    isa_ok( $parser->type($input),
        "Clownfish::Type::Composite", $input );
}

eval { my $type = Clownfish::Type::Composite->new };
like( $@, qr/child/i, "child required" );

my $foo_type = Clownfish::Type::Object->new( specifier => 'Foo' );
my $composite_type = Clownfish::Type::Composite->new(
    child       => $foo_type,
    indirection => 1,
);
is( $composite_type->get_specifier,
    'Foo', "get_specifier delegates to child" );

my $other = Clownfish::Type::Composite->new(
    child       => $foo_type,
    indirection => 1,
);
ok( $composite_type->equals($other), "equals" );
ok( $composite_type->is_composite,   "is_composite" );

my $bar_type = Clownfish::Type::Object->new( specifier => 'Bar' );
my $bar_composite = Clownfish::Type::Composite->new(
    child       => $bar_type,
    indirection => 1,
);
ok( !$composite_type->equals($bar_composite),
    "equals spoiled by different child"
);

my $foo_array = $parser->type("foo_t[]")
    or die "Can't parse foo_t[]";
is( $foo_array->get_array, '[]', "get_array" );
unlike( $foo_array->to_c, qr/\[\]/, "array subscripts not included by to_c" );

my $foo_array_array = $parser->type("foo_t[][]")
    or die "Can't parse foo_t[][]";
ok( !$foo_array->equals($foo_array_array),
    "equals spoiled by different array postfixes"
);

my $foo_star = $parser->type("foo_t*")
    or die "Can't parse foo_t*";
my $foo_star_star = $parser->type("foo_t**")
    or die "Can't parse foo_t**";
ok( !$foo_star->equals($foo_star_star),
    "equals spoiled by different levels of indirection" );
