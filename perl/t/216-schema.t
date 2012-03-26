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
use Lucy::Test;

package main;
use Test::More tests => 2;

my $schema;
SKIP: {
    skip( "constructor bailouts cause leaks", 1 ) if $ENV{LUCY_VALGRIND};

    $schema = Lucy::Test::TestSchema->new;
    eval { $schema->spec_field( name => 'foo', type => 'NotAType' ) };
    Test::More::like( $@, qr/FieldType/, "bogus FieldType fails to load" );
}

$schema = Lucy::Test::TestSchema->new;
my $type = $schema->fetch_type('content');
$schema->spec_field( name => 'new_field', type => $type );
my $got = grep { $_ eq 'new_field' } @{ $schema->all_fields };
ok( $got, 'spec_field works' );

