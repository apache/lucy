use strict;
use warnings;

use Config;
use Test::More;
BEGIN {
    if ( $Config{usethreads} ) {
        plan( tests => 8 );
    }
    else {
        plan( skip_all => 'No thread support' );
    }
}
use threads;
use threads::shared;

package MyHash;
use base qw( Lucy::Object::Hash );

our %foo;
my %bar;
our $num_created : shared = 0;
my $num_destroyed : shared = 0;

sub new {
    my ( $either, %args ) = @_;
    my $foo  = delete $args{foo};
    my $bar  = delete $args{bar};
    my $self = $either->SUPER::new(%args);
    $foo{$$self} = $foo;
    $bar{$$self} = $bar;
    $num_created++;
    return $self;
}

sub get_foo { $foo{ ${ +shift } } }
sub get_bar { $bar{ ${ +shift } } }

sub DESTROY {
    my $self = shift;
    delete $foo{$$self};
    delete $bar{$$self};
    $self->SUPER::DESTROY;
    $num_destroyed++;
}

package main;
use Time::HiRes qw( usleep );

# Establish the VTable for MyHash before we start threads, since the VTable
# registration process is still racy.
MyHash->new;
$num_created   = 0;
$num_destroyed = 0;
use Devel::Peek qw( SvREFCNT );

sub try_a_hash {
    my $number    = shift;
    my $test_hash = MyHash->new(
        foo => $number,
        bar => $number,
    );
    $test_hash->store( stuff => Lucy::Object::CharBuf->new("things") );
    die "store/fetch failed for $number"
        unless $test_hash->fetch("stuff") eq 'things';
    die "failed to store inside out var in package global hash"
        unless $test_hash->get_foo == $number;
    die "failed to store inside out var in lexical hash"
        unless $test_hash->get_bar == $number;
    my $vtable = $test_hash->get_vtable;
    usleep(100_000);
    undef $test_hash;
    return "$vtable";
}

my @threads;

for my $num ( 1 .. 5 ) {
    my $thread = threads->create( \&try_a_hash, $num );
    push @threads, $thread;
}

# Give threads time to finish creating the MyHash objects.
usleep(100_000);

is( $num_created, 5, "objects created inside threads" );
is( scalar keys %foo,
    0, "package global inside out vars not visible from other contexts" );
is( scalar keys %bar,
    0, "lexical inside out vars not visible from other contexts" );

my @stringified_vtable_refs = $_->join for @threads;
my @expected = ( $stringified_vtable_refs[0] ) x @stringified_vtable_refs;
is_deeply( \@stringified_vtable_refs, \@expected,
    "same Perl object for the VTable across multiple threads" );

is( $num_destroyed, 5, "objects destroyed inside threads" );

my $vtable = Lucy::Object::VTable::_get_registry->fetch("MyHash");
isa_ok( $vtable, "Lucy::Object::VTable",
    "Dynamically created vtable lives on after all objects destroyed" );
is( SvREFCNT($$vtable), 2,
    "Correct refcount for VTable after all threads cleaned up" );
is( $vtable->get_refcount, 1, "VTable_Get_RefCount lies" );

