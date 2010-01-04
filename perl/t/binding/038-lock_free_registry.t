use strict;
use warnings;

use Config;
use Test::More;
BEGIN {
    if ( $ENV{LUCY_VALGRIND} ) {
        plan( skip_all => 'Known leaks' );
    }
    elsif ( $Config{usethreads} ) {
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
use Lucy::Test;

my $registry = Lucy::Object::LockFreeRegistry->new( capacity => 32 );

sub register_many {
    my ( $nums, $delay ) = @_;

    # Encourage contention, so that all threads try to register at the same
    # time.
    sleep $delay;
    threads->yield();

    my $succeeded = 0;
    for my $number (@$nums) {
        my $obj = Lucy::Object::CharBuf->new($number);
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

