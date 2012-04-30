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

use Test::More tests => 6;
use Lucy::Test;

my $tokenizer = Lucy::Analysis::StandardTokenizer->new;
my $other     = Lucy::Analysis::StandardTokenizer->new;
ok( $tokenizer->equals($other), "Equals" );

my $text = $tokenizer->split("o'malley's")->[0];
is( $text, "o'malley's", "multiple apostrophes" );

my $inversion = Lucy::Analysis::Inversion->new( text => "a b c" );
$inversion = $tokenizer->transform($inversion);

my ( @token_texts, @start_offsets, @end_offsets );
while ( my $token = $inversion->next ) {
    push @token_texts,   $token->get_text;
    push @start_offsets, $token->get_start_offset;
    push @end_offsets,   $token->get_end_offset;
}
is_deeply( \@token_texts, [qw( a b c )], "correct texts" );
is_deeply( \@start_offsets, [ 0, 2, 4, ], "correctstart offsets" );
is_deeply( \@end_offsets,   [ 1, 3, 5, ], "correct end offsets" );

$inversion->reset;
$inversion   = $tokenizer->transform($inversion);
@token_texts = ();
while ( my $token = $inversion->next ) {
    push @token_texts, $token->get_text;
}
is_deeply(
    \@token_texts,
    [ 'a', 'b', 'c' ],
    "no freakout when fed multiple tokens"
);

