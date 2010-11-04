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

use Test::More skip_all => 'Disabled until test ported to C';
#use Test::More tests => 5;
use Lucy::Test;
use Lucy qw( to_perl );

my $letters = Lucy::Object::VArray->new( capacity => 26 );
$letters->push( Lucy::Object::ByteBuf->new($_) ) for 'a' .. 'z';
my $run = Lucy::Test::Util::BBSortEx->new( external => $letters );
$run->set_mem_thresh(5);

my $num_in_cache = $run->refill;
is( $run->cache_count, 5, "Read_Elem puts the brakes on Refill" );
my $endpost = $run->peek_last;
is( $endpost, 'e', "Peek_Last" );
$endpost = Lucy::Object::ByteBuf->new('b');
my $slice = $run->pop_slice($endpost);
is( scalar @$slice, 2, "Pop_Slice gets only less-than-or-equal elems" );
@$slice = map { to_perl($_) } @$slice;
is_deeply( $slice, [qw( a b )], "Pop_Slice picks highest elems" );

my @got = qw( a b );
while (1) {
    $endpost = $run->peek_last;
    $slice   = $run->pop_slice( Lucy::Object::ByteBuf->new($endpost) );
    push @got, map { to_perl($_) } @$slice;
    last unless $run->refill;
}
is_deeply( \@got, [ 'a' .. 'z' ], "retrieve all elems" );

