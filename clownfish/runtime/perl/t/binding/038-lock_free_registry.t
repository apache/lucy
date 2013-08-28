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

use Config;
use Test::More;
BEGIN {
    if ( $ENV{LUCY_VALGRIND} ) {
        plan( skip_all => 'Known leaks' );
    }
    elsif ( !defined( $ENV{LUCY_DEBUG} ) ) {
        plan( skip_all => 'Debug-only test' );
    }
    elsif ( $Config{usethreads} and $^O !~ /mswin/i ) {
        plan( tests => 1 );
    }
    else {
        plan( skip_all => 'No thread support' );
    }
}
use threads;
use threads::shared;
use Time::HiRes qw( time usleep );
use List::Util qw( shuffle );
use Clownfish;

my $registry = Clownfish::LockFreeRegistry->new( capacity => 32 );

sub register_many {
    my ( $nums, $delay ) = @_;

    # Encourage contention, so that all threads try to register at the same
    # time.
    sleep $delay;
    threads->yield();

    my $succeeded = 0;
    for my $number (@$nums) {
        my $obj = Clownfish::String->new($number);
        $succeeded += $registry->register( key => $obj, value => $obj );
    }

    return $succeeded;
}

my @threads;

my $target_time = time() + .5;
my @num_sets = map { [ shuffle( 1 .. 10000 ) ] } 1 .. 5;
for my $num ( 1 .. 5 ) {
    my $delay = $target_time - time();
    my $thread = threads->create( \&register_many, pop @num_sets, $delay );
    push @threads, $thread;
}

my $total_succeeded = 0;
$total_succeeded += $_->join for @threads;

is( $total_succeeded, 10000,
    "registered exactly the right number of entries across all threads" );

