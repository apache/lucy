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

use Test::More tests => 11;
use Clownfish::Type::Float;
use Clownfish::Parser;

my $float_type = Clownfish::Type::Float->new(
    specifier => 'float',
    const     => 1,
);
ok( $float_type->const, "const" );
is( $float_type->get_specifier, "float" );
like( $float_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::Parser->new;

for my $specifier (qw( float double)) {
    is( $parser->c_float_specifier($specifier),
        $specifier, "c_float_specifier: $specifier" );
    isa_ok( $parser->float_type($specifier), "Clownfish::Type::Float" );
    isa_ok( $parser->float_type("const $specifier"),
        "Clownfish::Type::Float" );
    my $bogus = $specifier . "y";
    ok( !$parser->c_float_specifier($bogus),
        "c_float_specifier guards against partial word matches" );
}

