use strict;
use warnings;

use Test::More tests => 13;

BEGIN { use_ok('Clownfish::ParamList') }
use Clownfish::Type;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
$parser->parcel_definition('parcel Neato;')
    or die "failed to process parcel_definition";

isa_ok( $parser->param_variable($_),
    "Clownfish::Variable", "param_variable: $_" )
    for ( 'u32_t baz', 'CharBuf *stuff', 'float **ptr', );

my $param_list = $parser->param_list("(Obj *self, int num)");
isa_ok( $param_list, "Clownfish::ParamList" );
ok( !$param_list->variadic, "not variadic" );
is( $param_list->to_c, 'neato_Obj* self, int num', "to_c" );
is( $param_list->name_list, 'self, num', "name_list" );

$param_list = $parser->param_list("(Obj *self=NULL, int num, ...)");
ok( $param_list->variadic, "variadic" );
is_deeply(
    $param_list->get_initial_values,
    [ "NULL", undef ],
    "initial_values"
);
is( $param_list->to_c, 'neato_Obj* self, int num, ...', "to_c" );
is( $param_list->num_vars, 2, "num_vars" );
isa_ok( $param_list->get_variables->[0],
    "Clownfish::Variable", "get_variables..." );

