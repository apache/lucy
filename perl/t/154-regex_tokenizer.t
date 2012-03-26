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

use Test::More tests => 15;
use Lucy::Test;

my $tokenizer   = Lucy::Analysis::RegexTokenizer->new;
my $other       = Lucy::Analysis::RegexTokenizer->new( pattern => '\w+' );
my $yet_another = Lucy::Analysis::RegexTokenizer->new( pattern => '\w+' );
ok( $other->equals($yet_another), "Equals" );
ok( !$tokenizer->equals($other),  "different patterns foil Equals" );

my $text = $tokenizer->split("o'malley's")->[0];
is( $text, "o'malley's", "multiple apostrophes for default pattern" );

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

$tokenizer = Lucy::Analysis::RegexTokenizer->new( pattern => '.' );
$inversion = Lucy::Analysis::Inversion->new( text => "a b c" );
$inversion = $tokenizer->transform($inversion);

@token_texts   = ();
@start_offsets = ();
@end_offsets   = ();
while ( my $token = $inversion->next ) {
    push @token_texts,   $token->get_text;
    push @start_offsets, $token->get_start_offset;
    push @end_offsets,   $token->get_end_offset;
}
is_deeply(
    \@token_texts,
    [ 'a', ' ', 'b', ' ', 'c' ],
    "texts: custom pattern"
);
is_deeply( \@start_offsets, [ 0 .. 4 ], "starts: custom pattern" );
is_deeply( \@end_offsets,   [ 1 .. 5 ], "ends: custom pattern" );

$inversion->reset;
$inversion   = $tokenizer->transform($inversion);
@token_texts = ();
while ( my $token = $inversion->next ) {
    push @token_texts, $token->get_text;
}
is_deeply(
    \@token_texts,
    [ 'a', ' ', 'b', ' ', 'c' ],
    "no freakout when fed multiple tokens"
);

$tokenizer = Lucy::Analysis::RegexTokenizer->new( token_re => qr/../ );
is_deeply( $tokenizer->split('aabbcc'),
    [qw( aa bb cc )], "back compat with token_re argument" );

eval {
    my $toke
        = Lucy::Analysis::RegexTokenizer->new(
        pattern => '\\p{Carp::confess}' );
};
like( $@, qr/\\p/, "\\p forbidden in pattern" );

eval {
    my $toke
        = Lucy::Analysis::RegexTokenizer->new(
        pattern => '\\P{Carp::confess}' );
};
like( $@, qr/\\P/, "\\P forbidden in pattern" );

$tokenizer = Lucy::Analysis::RegexTokenizer->new( pattern => '\\w+' );
my $dump = $tokenizer->dump;
$dump->{pattern} = "\\p{Carp::confess}";
eval { $tokenizer->load($dump) };
like( $@, qr/\\p/, "\\p forbidden during load" );

$dump->{pattern} = "\\P{Carp::confess}";
eval { $tokenizer->load($dump) };
like( $@, qr/\\P/, "\\P forbidden during load" );
