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

use Test::More tests => 5;
use Lucy::Test::TestUtils qw( utf8_test_strings test_analyzer );

package TestAnalyzer;
use base qw( Lucy::Analysis::Analyzer );
sub transform { $_[1] }    # satisfy mandatory override

package main;
my $analyzer = TestAnalyzer->new;

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

my $got = $analyzer->split($not_a_smiley)->[0];
is( $got, $frowny, "split() upgrades non-UTF-8 correctly" );

$got = $analyzer->split($smiley)->[0];
is( $got, $smiley, "split() handles UTF-8 correctly" );

test_analyzer( $analyzer, 'foo', ['foo'], "Analyzer (no-op)" );
