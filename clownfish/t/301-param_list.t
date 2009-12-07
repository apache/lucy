use strict;
use warnings;

use Test::More tests => 13;

BEGIN { use_ok('Boilerplater::ParamList') }
use Boilerplater::Type;
use Boilerplater::Parser;

my $parser = Boilerplater::Parser->new;
$parser->parcel_definition('parcel Boil;')
    or die "failed to process parcel_definition";

isa_ok( $parser->param_variable($_),
    "Boilerplater::Variable", "param_variable: $_" )
    for ( 'u32_t baz', 'CharBuf *stuff', 'float **ptr', );

my $param_list = $parser->param_list("(Obj *self, int num)");
isa_ok( $param_list, "Boilerplater::ParamList" );
ok( !$param_list->variadic, "not variadic" );
is( $param_list->to_c, 'boil_Obj* self, int num', "to_c" );
is( $param_list->name_list, 'self, num', "name_list" );

$param_list = $parser->param_list("(Obj *self=NULL, int num, ...)");
ok( $param_list->variadic, "variadic" );
is_deeply(
    $param_list->get_initial_values,
    [ "NULL", undef ],
    "initial_values"
);
is( $param_list->to_c, 'boil_Obj* self, int num, ...', "to_c" );
is( $param_list->num_vars, 2, "num_vars" );
isa_ok( $param_list->get_variables->[0],
    "Boilerplater::Variable", "get_variables..." );

