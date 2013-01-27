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
use Clownfish::CFC;

my $v3_2_1   = Clownfish::CFC::Model::Version->new( vstring => 'v3.2.1' );
my $v3_2     = Clownfish::CFC::Model::Version->new( vstring => 'v3.2' );
my $v3_3     = Clownfish::CFC::Model::Version->new( vstring => 'v3.3' );
my $v3_2_0   = Clownfish::CFC::Model::Version->new( vstring => 'v3.2.0' );
my $v3_2_1_0 = Clownfish::CFC::Model::Version->new( vstring => 'v3.2.1.0' );
my $v90210   = Clownfish::CFC::Model::Version->new( vstring => 'v90210' );

is( $v3_2_1->get_major,   3,        'get_major' );
is( $v90210->get_major,   90210,    'parse big number' );
is( $v3_2_1->get_vstring, 'v3.2.1', 'get_vstring' );

is( $v3_2_1->compare_to($v3_2_1_0), 0,  "ignore zeroes in compare_to" );
is( $v3_2_1_0->compare_to($v3_2_1), 0,  "ignore zeroes in compare_to" );
is( $v3_2_1->compare_to($v3_3),     -1, "compare_to A < B_fewer_digits" );
is( $v3_3->compare_to($v3_2_1),     1,  "compare_to A_fewer_digits > B" );
is( $v3_2_1->compare_to($v3_2),     1,  "compare_to A < B_fewer_digits" );
is( $v3_2->compare_to($v3_2_1),     -1, "compare_to A_fewer_digits > B" );
is( $v3_2_1->compare_to($v3_2_0),   1,  "compare_to A > B" );
is( $v3_2_0->compare_to($v3_2_1),   -1, "compare_to A < B" );

