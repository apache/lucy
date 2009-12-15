use strict;
use warnings;

use Test::More tests => 124;

BEGIN { use_ok('Clownfish::Parser') }

my $parser = Clownfish::Parser->new;
isa_ok( $parser, "Clownfish::Parser" );

isa_ok( $parser->parcel_definition("parcel Fish;"),
    "Clownfish::Parcel", "parcel_definition" );
isa_ok( $parser->parcel_definition("parcel Crustacean cnick Crust;"),
    "Clownfish::Parcel", "parcel_definition with cnick" );

# Set and leave parcel.
my $parcel = $parser->parcel_definition('parcel Crustacean cnick Crust;')
    or die "failed to process parcel_definition";
is( $Clownfish::Parser::parcel, $parcel,
    "parcel_definition sets internal \$parcel var" );

is( $parser->strip_plain_comments("/*x*/"),
    "     ", "comments replaced by spaces" );
is( $parser->strip_plain_comments("/**x*/"),
    "/**x*/", "docu-comment untouched" );
is( $parser->strip_plain_comments("/*\n*/"), "  \n  ", "newline preserved" );

for (qw( foo _foo foo_yoo FOO Foo fOO f00 )) {
    is( $parser->identifier($_), $_, "identifier: $_" );
}

for (qw( void unsigned float u32_t i64_t u8_t bool_t )) {
    ok( !$parser->identifier($_), "reserved word not an identifier: $_" );
}

is( $parser->chy_integer_specifier($_), $_, "Charmony integer specifier $_" )
    for qw( u8_t u16_t u32_t u64_t i8_t i16_t i32_t i64_t bool_t );

is( $parser->object_type_specifier($_), $_, "object_type_specifier $_" )
    for qw( ByteBuf Obj ANDScorer );

is( $parser->type_specifier($_), $_, "type_specifier $_" )
    for qw( u32_t char int short long float double void ANDScorer );

is( $parser->type_qualifier($_), $_, "type_qualifier $_" ) for qw( const );

is( $parser->exposure_specifier($_), $_, "exposure_specifier $_" )
    for qw( public private parcel );

is( $parser->type_postfix($_), $_, "postfix: $_" )
    for ( '[]', '[A_CONSTANT]', '*' );
is( $parser->type_postfix('[ FOO ]'), '[FOO]', "type_postfix: [ FOO ]" );

isa_ok( $parser->type($_), "Clownfish::Type", "type $_" )
    for ( 'const char *', 'Obj*', 'i32_t', 'char[]', 'long[1]',
    'i64_t[FOO]' );

is( $parser->declarator($_), $_, "declarator: $_" )
    for ( 'foo', 'bar_bar_bar' );

isa_ok( $parser->param_variable($_),
    "Clownfish::Variable", "param_variable: $_" )
    for ( 'u32_t baz;', 'CharBuf *stuff;', 'float **ptr;', );

isa_ok( $parser->var_declaration_statement($_)->{declared},
    "Clownfish::Variable", "var_declaration_statement: $_" )
    for (
    'parcel int foo;',
    'private Obj *obj;',
    'public inert i32_t **foo;',
    'Dog *fido;'
    );

is( $parser->hex_constant($_), $_, "hex_constant: $_" )
    for (qw( 0x1 0x0a 0xFFFFFFFF ));

is( $parser->integer_constant($_), $_, "integer_constant: $_" )
    for (qw( 1 -9999  0 10000 ));

is( $parser->float_constant($_), $_, "float_constant: $_" )
    for (qw( 1.0 -9999.999  0.1 0.0 ));

is( $parser->string_literal($_), $_, "string_literal: $_" )
    for ( q|"blah"|, q|"blah blah"|, q|"\\"blah\\" \\"blah\\""| );

is( $parser->scalar_constant($_), $_, "scalar_constant: $_" )
    for ( q|"blah"|, 1, 1.2, "0xFC" );

my @composites = ( 'int[]', "i32_t **", "Foo **", "Foo ***", "const void *" );
for my $composite (@composites) {
    my $parsed = $parser->type($composite);
    isa_ok(
        $parsed,
        "Clownfish::Type::Composite",
        "composite_type: $composite"
    );
}

my @object_types = ( 'Obj *', "incremented Foo*", "decremented CharBuf *" );
for my $object_type (@object_types) {
    my $parsed = $parser->object_type($object_type);
    isa_ok( $parsed, "Clownfish::Type::Object",
        "object_type: $object_type" );
}

my %param_lists = (
    '(int foo)'                 => 1,
    '(Obj *foo, Foo **foo_ptr)' => 2,
    '()'                        => 0,
);
while ( my ( $param_list, $num_params ) = each %param_lists ) {
    my $parsed = $parser->param_list($param_list);
    isa_ok( $parsed, "Clownfish::ParamList", "param_list: $param_list" );
}
ok( $parser->param_list("(int foo, ...)")->variadic, "variadic param list" );
my $param_list = $parser->param_list(q|(int foo = 0xFF, char *bar ="blah")|);
is_deeply(
    $param_list->get_initial_values,
    [ '0xFF', '"blah"' ],
    "initial values"
);

my %sub_args = ( class => 'Stuff::Obj', cnick => 'Obj' );

ok( $parser->declaration_statement( $_, 0, %sub_args, inert => 1 ),
    "declaration_statment: $_" )
    for (
    'public Foo* Spew_Foo(Obj *self, u32_t *how_many);',
    'private Hash *hash;',
    );

is( $parser->object_type_specifier($_), $_, "object_type_specifier: $_" )
    for (qw( Foo FooJr FooIII Foo4th ));

ok( !$parser->object_type_specifier($_), "illegal object_type_specifier: $_" )
    for (qw( foo fooBar Foo_Bar FOOBAR 1Foo 1FOO ));

is( $parser->class_name($_), $_, "class_name: $_" )
    for (qw( Foo Foo::FooJr Foo::FooJr::FooIII Foo::FooJr::FooIII::Foo4th ));

ok( !$parser->class_name($_), "illegal class_name: $_" )
    for (qw( foo fooBar Foo_Bar FOOBAR 1Foo 1FOO ));

is( $parser->cnick(qq|cnick $_|), $_, "cnick: $_" ) for (qw( Foo FF ));

ok( !$parser->cnick(qq|cnick $_|), "Illegal cnick: $_" )
    for (qw( foo fOO 1Foo ));

