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

use Test::More tests => 15;
use Clownfish::Type;
use Clownfish::Parser;

my $float_type = Clownfish::Type->new_float(
    specifier => 'float',
    const     => 1,
);
ok( $float_type->const, "const" );
is( $float_type->get_specifier, "float" );
like( $float_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::Parser->new;

for my $specifier (qw( float double)) {
    my $type = $parser->parse($specifier);
    isa_ok( $type, "Clownfish::Type" );
    ok( $type && $type->is_floating, "parsed specifier is_floating()" );
    $type = $parser->parse("const $specifier");
    isa_ok( $type, "Clownfish::Type" );
    ok( $type && $type->is_floating, "parsed const specifier is_floating()" );
    ok( $type && $type->const,       "parsed const specifier is_floating()" );
SKIP: {
        skip( "No way to catch parser exception at present", 1 );
        my $bogus = $specifier . "y";
        ok( !$parser->parse($bogus),
            "c_float_specifier guards against partial word matches" );
    }
}

