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

use Test::More tests => 4;
use Clownfish::CFC::Model::Type;
use Clownfish::CFC::Parser;

my $va_list_type = Clownfish::CFC::Model::Type->new_va_list;
is( $va_list_type->get_specifier,
    "va_list", "specifier defaults to 'va_list'" );
is( $va_list_type->to_c, "va_list", "to_c" );

my $parser = Clownfish::CFC::Parser->new;

my $type = $parser->parse('va_list');
ok( $type && $type->is_va_list, "parse va_list" );
SKIP: {
    skip( "No way to catch parser exception at present", 1 );
    ok( !$parser->parse('va_listable'),
        "va_list_type_specifier guards against partial word matches" );
}

