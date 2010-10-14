use strict;
use warnings;
use lib 'buildlib';

use Time::HiRes qw( sleep );
use Test::More;
use File::Spec::Functions qw( catfile );
use KinoSearch::Test::TestUtils qw( init_test_index_loc );

BEGIN {
    if ( $^O =~ /mswin/i ) {
        plan( 'skip_all', "fork on Windows not supported by KS" );
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

    my $folder = KinoSearch::Store::FSFolder->new( path => $path );

    sub make_lock {
        my $lock = KinoSearch::Store::LockFileLock->new(
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
