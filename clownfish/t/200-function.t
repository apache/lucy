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

use Test::More tests => 6;

BEGIN { use_ok('Clownfish::Function') }
use Clownfish::Parser;
use Clownfish::Parcel;

my $parser = Clownfish::Parser->new;
$parser->parcel_definition('parcel Neato;')
    or die "failed to process parcel_definition";

my %args = (
    parcel      => 'Neato',
    return_type => $parser->type('Obj*'),
    class_name  => 'Neato::Foo',
    class_cnick => 'Foo',
    param_list  => $parser->param_list('(int32_t some_num)'),
    micro_sym   => 'return_an_obj',
);

my $func = Clownfish::Function->new(%args);
isa_ok( $func, "Clownfish::Function" );

eval { my $death = Clownfish::Function->new( %args, extra_arg => undef ) };
like( $@, qr/extra_arg/, "Extra arg kills constructor" );

eval { Clownfish::Function->new( %args, micro_sym => 'Uh_Oh' ); };
like( $@, qr/Uh_Oh/, "invalid micro_sym kills constructor" );

my %sub_args = ( class => 'Neato::Obj', cnick => 'Obj' );

isa_ok(
    $parser->subroutine_declaration_statement( $_, 0, %sub_args, inert => 1 )
        ->{declared},
    "Clownfish::Function",
    "function declaration: $_"
    )
    for (
    'inert int running_count(int biscuit);',
    'public inert Hash* init_fave_hash(int32_t num_buckets, bool_t o_rly);',
    );
