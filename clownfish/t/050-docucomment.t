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

use Test::More tests => 10;

BEGIN { use_ok('Clownfish::DocuComment') }
use Clownfish::Parser;

my $parser = Clownfish::Parser->new;
isa_ok( $parser->parse('/** foo. */'), "Clownfish::DocuComment" );

my $text = <<'END_COMMENT';
/**
 * Brief description.  Long description.
 *
 * More long description.
 *
 * @param foo A foo.
 * @param bar A bar.
 *
 * @param baz A baz.
 * @return a return value.
 */
END_COMMENT

my $docucomment = $parser->parse($text);

like(
    $docucomment->get_description,
    qr/^Brief.*long description.\s*\Z/ims,
    "get_description"
);
is( $docucomment->get_brief, "Brief description.", "brief" );
like( $docucomment->get_long, qr/^Long.*long description.\s*\Z/ims, "long" );
is_deeply( $docucomment->get_param_names, [qw( foo bar baz )],
    "param names" );
is( $docucomment->get_param_docs->[0], "A foo.", '@param terminated by @' );
is( $docucomment->get_param_docs->[1],
    "A bar.", '@param terminated by empty line' );
is( $docucomment->get_param_docs->[2],
    "A baz.", '@param terminated next element, @return' );
is( $docucomment->get_retval, "a return value.", "get_retval" );

