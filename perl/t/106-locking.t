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

use Time::HiRes qw( sleep );
use Test::More;
use File::Spec::Functions qw( catfile );
use Lucy::Test::TestUtils qw( init_test_index_loc );

BEGIN {
    if ( $^O =~ /(mswin|cygwin)/i ) {
        plan( 'skip_all', "fork on Windows not supported by Lucy" );
    }
    else {
        plan( tests => 3 );
    }
}

my $path = init_test_index_loc();

Dead_locks_are_removed: {
    my $lock_path = catfile( $path, 'locks', 'foo.lock' );

    # Remove any existing lockfile
    unlink $lock_path;
    die "Can't unlink '$lock_path'" if -e $lock_path;

    my $folder = Lucy::Store::FSFolder->new( path => $path );

    sub make_lock {
        my $lock = Lucy::Store::LockFileLock->new(
            timeout => 0,
            name    => 'foo',
            host    => '',
            @_
        );
        $lock->clear_stale;
        $lock->obtain or die "no dice";
        return $lock;
    }

    # Fork a process that will create a lock and then exit
    my $pid = fork();
    if ( $pid == 0 ) {    # child
        make_lock( folder => $folder );
        exit;
    }
    else {
        waitpid( $pid, 0 );
    }

    sleep .1;
    ok( -e $lock_path, "child secured lock" );

    # The locking attempt will fail if the pid from the process that made the
    # lock is active, so do the best we can to see whether another process
    # started up with the child's pid (which would be weird).
    my $pid_active = kill( 0, $pid );

    eval { make_lock( folder => $folder, host => 'somebody_else' ) };
    like( $@, qr/no dice/, "different host fails to get lock" );

    eval { make_lock( folder => $folder ) };
    warn $@ if $@;
    my $saved_err = $@;
    $pid_active ||= kill( 0, $pid );
SKIP: {
        skip( "Child's pid is active", 1 ) if $pid_active;
        ok( !$saved_err,
            'second lock attempt clobbered dead lock file and did not die' );
    }

    undef $folder;
}
