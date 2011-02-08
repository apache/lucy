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

use Test::More tests => 12;
use Clownfish::Type;
use Clownfish::Parser;

my $void_type = Clownfish::Type->new_void;
is( $void_type->get_specifier, "void", "specifier defaults to 'void'" );
is( $void_type->to_c,          "void", "to_c" );
ok( $void_type->is_void, "is_void" );

$void_type = Clownfish::Type->new_void(
    specifier => 'void',
    const     => 1,
);
ok( $void_type->const, "const" );
like( $void_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::Parser->new;

is( $parser->void_type_specifier('void'), 'void', 'void_type_specifier' );
$void_type = $parser->void_type('void');
isa_ok( $void_type, "Clownfish::Type" );
ok( $void_type && $void_type->is_void,
    "Parser calls new_void() when parsing 'void'" );
my $const_void_type = $parser->void_type('const void');
isa_ok( $const_void_type, "Clownfish::Type" );
ok( $const_void_type && $const_void_type->is_void,
    "Parser calls new_void() when parsing 'const void'"
);
ok( $const_void_type && $const_void_type->const,
    "Parser preserves const when parsing 'const void'"
);
ok( !$parser->void_type_specifier('voidable'),
    "void_type_specifier guards against partial word matches" );

