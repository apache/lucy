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

use Test::More tests => 5;

use Clownfish::CBlock;
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;

my $block = Clownfish::CBlock->new( contents => 'int foo;' );
isa_ok( $block, "Clownfish::CBlock" );
is( $block->get_contents, 'int foo;', "get_contents" );
eval { Clownfish::CBlock->new };
like( $@, qr/contents/, "content required" );

$block = $parser->parse(qq| __C__\n#define FOO_BAR 1\n__END_C__  |);

isa_ok( $block, "Clownfish::CBlock" );
is( $block->get_contents, "#define FOO_BAR 1\n", "parse embed_c" );

