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
use lib 'buildlib';

use Test::More tests => 6;

use Lucy::Test;
use Lucy::Search::Span;

my $span = Lucy::Search::Span->new(
    offset => 2,
    length => 3,
    weight => 7,
);

is( $span->get_offset, 2, "get_offset" );
is( $span->get_length, 3, "get_length" );
is( $span->get_weight, 7, "get_weight" );

$span->set_offset(10);
$span->set_length(1);
$span->set_weight(4);

is( $span->get_offset, 10, "set_offset" );
is( $span->get_length, 1,  "set_length" );
is( $span->get_weight, 4,  "set_weight" );

