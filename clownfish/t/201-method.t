use strict;
use warnings;

use Test::More tests => 30;

BEGIN { use_ok('Clownfish::Method') }
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
$parser->parcel_definition('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    parcel      => 'Neato',
    return_type => $parser->type('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(Foo *self, int32_t count = 0)'),
    macro_sym   => 'Return_An_Obj',
);

my $method = Clownfish::Method->new(%args);
isa_ok( $method, "Clownfish::Method" );

ok( $method->parcel, "parcel exposure by default" );

eval { my $death = Clownfish::Method->new( %args, extra_arg => undef ) };
like( $@, qr/extra_arg/, "Extra arg kills constructor" );

eval { Clownfish::Method->new( %args, macro_sym => 'return_an_obj' ); };
like( $@, qr/macro_sym/, "Invalid macro_sym kills constructor" );

my $dupe = Clownfish::Method->new(%args);
ok( $method->compatible($dupe), "compatible()" );

my $macro_sym_differs = Clownfish::Method->new( %args, macro_sym => 'Eat' );
ok( !$method->compatible($macro_sym_differs),
    "different macro_sym spoils compatible()"
);
ok( !$macro_sym_differs->compatible($method), "... reversed" );

my $extra_param = Clownfish::Method->new( %args,
    param_list =>
        $parser->param_list('(Foo *self, int32_t count = 0, int b)'), );
ok( !$method->compatible($macro_sym_differs),
    "extra param spoils compatible()"
);
ok( !$extra_param->compatible($method), "... reversed" );

my $default_differs = Clownfish::Method->new( %args,
    param_list => $parser->param_list('(Foo *self, int32_t count = 1)'), );
ok( !$method->compatible($default_differs),
    "different initial_value spoils compatible()"
);
ok( !$default_differs->compatible($method), "... reversed" );

my $missing_default = Clownfish::Method->new( %args,
    param_list => $parser->param_list('(Foo *self, int32_t count)'), );
ok( !$method->compatible($missing_default),
    "missing initial_value spoils compatible()"
);
ok( !$missing_default->compatible($method), "... reversed" );

my $param_name_differs = Clownfish::Method->new( %args,
    param_list => $parser->param_list('(Foo *self, int32_t countess)'), );
ok( !$method->compatible($param_name_differs),
    "different param name spoils compatible()"
);
ok( !$param_name_differs->compatible($method), "... reversed" );

my $param_type_differs = Clownfish::Method->new( %args,
    param_list => $parser->param_list('(Foo *self, uint32_t count)'), );
ok( !$method->compatible($param_type_differs),
    "different param type spoils compatible()"
);
ok( !$param_type_differs->compatible($method), "... reversed" );

my $self_type_differs = Clownfish::Method->new(
    %args,
    class_name  => 'Neato::Bar',
    class_cnick => 'Bar',
    param_list  => $parser->param_list('(Bar *self, int32_t count = 0)'),
);
ok( $method->compatible($self_type_differs),
    "different self type still compatible(), since can't test inheritance" );
ok( $self_type_differs->compatible($method), "... reversed" );

my $not_final = Clownfish::Method->new(%args);
my $final     = $not_final->finalize;

eval { $method->override($final); };
like( $@, qr/final/i, "Can't override final method" );

delete $not_final->{final};
delete $final->{final};
is_deeply( $not_final, $final, "Finalize clones properly" );

for my $meth_meth (qw( short_method_sym full_method_sym full_offset_sym)) {
    eval { my $blah = $method->$meth_meth; };
    like( $@, qr/invoker/, "$meth_meth requires invoker" );
}

my %sub_args = ( class => 'Neato::Obj', cnick => 'Obj' );

isa_ok(
    $parser->subroutine_declaration_statement( $_, 0, %sub_args )->{declared},
    "Clownfish::Method",
    "method declaration: $_"
    )
    for (
    'public int Do_Foo(Obj *self);',
    'parcel Obj* Gimme_An_Obj(Obj *self);',
    'void Do_Whatever(Obj *self, uint32_t a_num, float real);',
    'private Foo* Fetch_Foo(Obj *self, int num);',
    );

ok( $parser->subroutine_declaration_statement( $_, 0, %sub_args )->{declared}
        ->final,
    "final method: $_"
) for ( 'public final void The_End(Obj *self);', );

