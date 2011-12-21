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

use Test::More tests => 87;
use Clownfish::CFC::Type;
use Clownfish::CFC::Parser;

my $integer_type = Clownfish::CFC::Type->new_integer(
    specifier => 'int32_t',
    const     => 1,
);
ok( $integer_type->const, "const" );
is( $integer_type->get_specifier, "int32_t" );
like( $integer_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::CFC::Parser->new;

my @chy_specifiers = qw(
    bool_t
);
my @c_specifiers = qw(
    char
    short
    int
    long
    size_t
    int8_t
    int16_t
    int32_t
    int64_t
    uint8_t
    uint16_t
    uint32_t
    uint64_t
);

for my $chy_specifier (@chy_specifiers) {
    my $type = $parser->parse($chy_specifier);
    isa_ok( $type, "Clownfish::CFC::Type" );
    ok( $type && $type->is_integer, "parsed Type is_integer()" );
    $type = $parser->parse("const $chy_specifier");
    isa_ok( $type, "Clownfish::CFC::Type" );
    ok( $type && $type->is_integer, "parsed const Type is_integer()" );
    ok( $type && $type->const,      "parsed const Type is const()" );
SKIP: {
        skip( "No way to catch parser exception at present", 1 );
        my $bogus = $chy_specifier . "oot_toot";
        ok( !$parser->parse($bogus),
            "chy_integer_specifier guards against partial word matches" );
    }
}

for my $c_specifier (@c_specifiers) {
    my $type = $parser->parse($c_specifier);
    isa_ok( $type, "Clownfish::CFC::Type" );
    ok( $type && $type->is_integer, "parsed Type is_integer()" );
    $type = $parser->parse("const $c_specifier");
    isa_ok( $type, "Clownfish::CFC::Type" );
    ok( $type && $type->is_integer, "parsed const Type is_integer()" );
    ok( $type && $type->const,      "parsed const Type is const()" );
SKIP: {
        skip( "No way to catch parser exception at present", 1 );
        my $bogus = $c_specifier . "y";
        ok( !$parser->parse($bogus),
            "c_integer_specifier guards against partial word matches" );
    }
}
