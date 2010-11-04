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

use Test::More tests => 4;
use Lucy::Test::TestUtils qw( utf8_test_strings );

my $inversion = Lucy::Analysis::Inversion->new;
$inversion->append(
    Lucy::Analysis::Token->new(
        text         => "car",
        start_offset => 0,
        end_offset   => 3,
    ),
);
$inversion->append(
    Lucy::Analysis::Token->new(
        text         => "bike",
        start_offset => 10,
        end_offset   => 14,
    ),
);
$inversion->append(
    Lucy::Analysis::Token->new(
        text         => "truck",
        start_offset => 20,
        end_offset   => 25,
    ),
);

my @texts;
while ( my $token = $inversion->next ) {
    push @texts, $token->get_text;
}
is_deeply( \@texts, [qw( car bike truck )], "return tokens in order" );

$inversion = Lucy::Analysis::Inversion->new;
$inversion->append(
    Lucy::Analysis::Token->new(
        text         => "foo",
        start_offset => 0,
        end_offset   => 3,
        pos_inc      => 10,
    ),
);
$inversion->append(
    Lucy::Analysis::Token->new(
        text         => "bar",
        start_offset => 4,
        end_offset   => 7,
        pos_inc      => ( 2**31 - 2 ),
    ),
);
eval { $inversion->invert; };
like( $@, qr/position/, "catch overflow in token position calculation" );

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

$inversion = Lucy::Analysis::Inversion->new( text => $smiley );
is( $inversion->next->get_text,
    $smiley, "Inversion->new handles UTF-8 correctly" );
$inversion = Lucy::Analysis::Inversion->new( text => $not_a_smiley );
is( $inversion->next->get_text,
    $frowny, "Inversion->new upgrades non-UTF-8 correctly" );
