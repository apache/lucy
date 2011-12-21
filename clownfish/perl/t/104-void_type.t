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
use Clownfish::CFC::Type;
use Clownfish::CFC::Parser;

my $void_type = Clownfish::CFC::Type->new_void;
is( $void_type->get_specifier, "void", "specifier defaults to 'void'" );
is( $void_type->to_c,          "void", "to_c" );
ok( $void_type->is_void, "is_void" );

$void_type = Clownfish::CFC::Type->new_void( const => 1 );
ok( $void_type->const, "const" );
like( $void_type->to_c, qr/const/, "'const' in C representation" );

my $parser = Clownfish::CFC::Parser->new;

$void_type = $parser->parse('void');
isa_ok( $void_type, "Clownfish::CFC::Type" );
ok( $void_type && $void_type->is_void,
    "Parser calls new_void() when parsing 'void'" );
my $const_void_type = $parser->parse('const void');
isa_ok( $const_void_type, "Clownfish::CFC::Type" );
ok( $const_void_type && $const_void_type->is_void,
    "Parser calls new_void() when parsing 'const void'"
);
ok( $const_void_type && $const_void_type->const,
    "Parser preserves const when parsing 'const void'"
);

SKIP: {
    skip( "No way to catch parser exception at present", 1 );
    ok( !$parser->parse('voidable'),
        "void_type_specifier guards against partial word matches" );
}

