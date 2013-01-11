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
use List::Util qw( shuffle );
use Lucy::Test;
use bytes qw();

my ( $sortex, $buffer, @orig, @sort_output );

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 4 );
$sortex->feed( new_bytebuf('c') );
is( $sortex->buffer_count, 1, "feed elem into buffer" );

$sortex->feed( new_bytebuf('b') );
$sortex->feed( new_bytebuf('d') );
$sortex->sort_buffer;
SKIP: {
    skip( "Restore when porting test to C", 1 );
    $buffer = $sortex->_peek_buffer;
    is_deeply( $buffer, [qw( b c d )], "sort buffer" );
}

$sortex->feed( new_bytebuf('a') );
is( $sortex->buffer_count, 0,
    "buffer flushed automatically when mem_thresh crossed" );
#is( $sortex->get_num_runs, 1, "run added" );

my @bytebufs = map { new_bytebuf($_) } qw( x y z );
my $run = Lucy::Util::BBSortEx->new( external => \@bytebufs );
$sortex->add_run($run);
$sortex->flip;
@orig = qw( a b c d x y z );
while ( my $result = $sortex->fetch ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "Add_Run" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 4 );
$sortex->feed( new_bytebuf('c') );
$sortex->clear_buffer;
is( $sortex->buffer_count, 0, "Clear_Buffer" );
$sortex->feed( new_bytebuf('b') );
$sortex->feed( new_bytebuf('a') );
$sortex->flush;
$sortex->flip;
@orig = qw( a b );
is( $sortex->peek, 'a', "Peek" );

while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig,
    "elements cleared via Clear_Buffer truly cleared" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new;
@orig   = ( 'a' .. 'z' );
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "sort letters" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new;
@orig   = qw( a a a b c d x x x x x x y y );
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "sort repeated letters" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new;
@orig = ( '', '', 'a' .. 'z' );
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "sort letters and empty strings" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 30 );
@orig = 'a' .. 'z';
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "... with an absurdly low mem_thresh" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 1 );
@orig = 'a' .. 'z';
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $result = $sortex->fetch ) ) {
    push @sort_output, $result;
}
is_deeply( \@sort_output, \@orig, "... with an even lower mem_thresh" );
@orig        = ();
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new;
$sortex->flip;
@sort_output = $sortex->fetch;
is_deeply( \@sort_output, [undef], "Sorting nothing returns undef" );
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 5_000 );
@orig = map { pack( 'N', $_ ) } ( 0 .. 11_000 );
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
$sortex->flip;
while ( defined( my $item = $sortex->fetch ) ) {
    push @sort_output, $item;
}
is_deeply( \@sort_output, \@orig, "Sorting packed integers..." );
@sort_output = ();

$sortex = Lucy::Util::BBSortEx->new( mem_thresh => 15_000 );
@orig = ();
for my $iter ( 0 .. 1_000 ) {
    my $string = '';
    for my $string_len ( 0 .. int( rand(1200) ) ) {
        $string .= pack( 'C', int( rand(256) ) );
    }
    push @orig, $string;
}
$sortex->feed( new_bytebuf($_) ) for shuffle(@orig);
@orig = sort @orig;
$sortex->flip;
while ( defined( my $item = $sortex->fetch ) ) {
    push @sort_output, $item;
}
is_deeply( \@sort_output, \@orig, "Random binary strings of random length" );
@sort_output = ();

sub new_bytebuf { Clownfish::ByteBuf->new(shift) }

