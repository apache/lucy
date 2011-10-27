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

use Test::More tests => 13;

BEGIN { use_ok('Clownfish::ParamList') }
use Clownfish::Type;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
$parser->parse('parcel Neato;')
    or die "failed to process parcel_definition";

isa_ok( $parser->parse($_),
    "Clownfish::Variable", "param_variable: $_" )
    for ( 'uint32_t baz', 'CharBuf *stuff', 'float **ptr', );

my $param_list = $parser->parse("(Obj *self, int num)");
isa_ok( $param_list, "Clownfish::ParamList" );
ok( !$param_list->variadic, "not variadic" );
is( $param_list->to_c, 'neato_Obj* self, int num', "to_c" );
is( $param_list->name_list, 'self, num', "name_list" );

$param_list = $parser->parse("(Obj *self=NULL, int num, ...)");
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

