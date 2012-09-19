# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

use strict;
use warnings;

use Test::More tests => 30;

BEGIN { use_ok('Clownfish::CFC::Model::Method') }
use Clownfish::CFC::Parser;

my $parser = Clownfish::CFC::Parser->new;
$parser->parse('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    parcel      => 'Neato',
    return_type => $parser->parse('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->parse('(Foo *self, int32_t count = 0)'),
    macro_sym   => 'Return_An_Obj',
);

my $method = Clownfish::CFC::Model::Method->new(%args);
isa_ok( $method, "Clownfish::CFC::Model::Method" );

ok( $method->parcel, "parcel exposure by default" );

eval {
    my $death
        = Clownfish::CFC::Model::Method->new( %args, extra_arg => undef );
};
like( $@, qr/extra_arg/, "Extra arg kills constructor" );

eval {
    Clownfish::CFC::Model::Method->new( %args, macro_sym => 'return_an_obj' );
};
like( $@, qr/macro_sym/, "Invalid macro_sym kills constructor" );

my $dupe = Clownfish::CFC::Model::Method->new(%args);
ok( $method->compatible($dupe), "compatible()" );

my $macro_sym_differs
    = Clownfish::CFC::Model::Method->new( %args, macro_sym => 'Eat' );
ok( !$method->compatible($macro_sym_differs),
    "different macro_sym spoils compatible()"
);
ok( !$macro_sym_differs->compatible($method), "... reversed" );

my $extra_param = Clownfish::CFC::Model::Method->new( %args,
    param_list => $parser->parse('(Foo *self, int32_t count = 0, int b)'), );
ok( !$method->compatible($macro_sym_differs),
    "extra param spoils compatible()"
);
ok( !$extra_param->compatible($method), "... reversed" );

my $default_differs = Clownfish::CFC::Model::Method->new( %args,
    param_list => $parser->parse('(Foo *self, int32_t count = 1)'), );
ok( !$method->compatible($default_differs),
    "different initial_value spoils compatible()"
);
ok( !$default_differs->compatible($method), "... reversed" );

my $missing_default = Clownfish::CFC::Model::Method->new( %args,
    param_list => $parser->parse('(Foo *self, int32_t count)'), );
ok( !$method->compatible($missing_default),
    "missing initial_value spoils compatible()"
);
ok( !$missing_default->compatible($method), "... reversed" );

my $param_name_differs = Clownfish::CFC::Model::Method->new( %args,
    param_list => $parser->parse('(Foo *self, int32_t countess)'), );
ok( !$method->compatible($param_name_differs),
    "different param name spoils compatible()"
);
ok( !$param_name_differs->compatible($method), "... reversed" );

my $param_type_differs = Clownfish::CFC::Model::Method->new( %args,
    param_list => $parser->parse('(Foo *self, uint32_t count)'), );
ok( !$method->compatible($param_type_differs),
    "different param type spoils compatible()"
);
ok( !$param_type_differs->compatible($method), "... reversed" );

my $self_type_differs = Clownfish::CFC::Model::Method->new(
    %args,
    class_name  => 'Neato::Bar',
    class_cnick => 'Bar',
    param_list  => $parser->parse('(Bar *self, int32_t count = 0)'),
);
ok( $method->compatible($self_type_differs),
    "different self type still compatible(), since can't test inheritance" );
ok( $self_type_differs->compatible($method), "... reversed" );

my $not_final = Clownfish::CFC::Model::Method->new(%args);
my $final     = $not_final->finalize;

eval { $method->override($final); };
like( $@, qr/final/i, "Can't override final method" );

ok( $not_final->compatible($final), "Finalize clones properly" );

for my $meth_meth (qw( short_method_sym full_method_sym full_offset_sym)) {
    eval { my $blah = $method->$meth_meth; };
    like( $@, qr/invoker/, "$meth_meth requires invoker" );
}

$parser->set_class_name("Neato::Obj");
$parser->set_class_cnick("Obj");
isa_ok(
    $parser->parse($_),
    "Clownfish::CFC::Model::Method",
    "method declaration: $_"
    )
    for (
    'public int Do_Foo(Obj *self);',
    'parcel Obj* Gimme_An_Obj(Obj *self);',
    'void Do_Whatever(Obj *self, uint32_t a_num, float real);',
    'private Foo* Fetch_Foo(Obj *self, int num);',
    );

for ( 'public final void The_End(Obj *self);', ) {
    my $meth = $parser->parse($_);
    ok( $meth && $meth->final, "final method: $_" );
}

