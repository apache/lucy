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
use Clownfish::CFC::Type;
use Clownfish::CFC::Parser;

BEGIN { use_ok('Clownfish::CFC::Variable') }

my $parser = Clownfish::CFC::Parser->new;
$parser->parse('parcel Neato;')
    or die "failed to process parcel_definition";

sub new_type { $parser->parse(shift) }

eval {
    my $death = Clownfish::CFC::Variable->new(
        micro_sym => 'foo',
        type      => new_type('int'),
        extra_arg => undef,
    );
};
like( $@, qr/extra_arg/, "Extra arg kills constructor" );

eval { my $death = Clownfish::CFC::Variable->new( micro_sym => 'foo' ) };
like( $@, qr/type/, "type is required" );
eval { my $death = Clownfish::CFC::Variable->new( type => new_type('int32_t') ) };
like( $@, qr/micro_sym/, "micro_sym is required" );

my $var = Clownfish::CFC::Variable->new(
    micro_sym => 'foo',
    type      => new_type('float*')
);
is( $var->local_c,           'float* foo',  "local_c" );
is( $var->local_declaration, 'float* foo;', "declaration" );
ok( $var->local, "default to local access" );

$var = Clownfish::CFC::Variable->new(
    micro_sym => 'foo',
    type      => new_type('float[1]')
);
is( $var->local_c, 'float foo[1]',
    "to_c appends array to var name rather than type specifier" );

$var = Clownfish::CFC::Variable->new(
    parcel      => 'Neato',
    micro_sym   => 'foo',
    type        => new_type("Foo*"),
    class_name  => 'Crustacean::Lobster::LobsterClaw',
    class_cnick => 'LobClaw',
);
is( $var->global_c, 'neato_Foo* neato_LobClaw_foo', "global_c" );

isa_ok( $parser->parse($_), "Clownfish::CFC::Variable",
    "var_declaration_statement: $_" )
    for (
    'parcel int foo;',
    'private Obj *obj;',
    'public inert int32_t **foo;',
    'Dog *fido;'
    );
