use strict;
use warnings;

use Test::More tests => 7;

package BasicObj;
use base qw( KinoSearch::Object::Obj );

package MyObj;
use base qw( KinoSearch::Object::Obj );

my %extra;

sub get_extra { my $self = shift; $extra{$$self} }

sub new {
    my ( $class, $extra ) = @_;
    my $self = $class->SUPER::new();
    $extra{$$self} = $extra;
    return $self;
}

sub serialize {
    my ( $self, $outstream ) = @_;
    $self->SUPER::serialize($outstream);
    $outstream->write_string( $self->get_extra );
}

sub deserialize {
    my ( $thing, $instream ) = @_;
    my $self = $thing->SUPER::deserialize($instream);
    $extra{$$self} = $instream->read_string;
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $extra{$$self};
    $self->SUPER::DESTROY;
}

package BadObj;
use base qw( MyObj );

sub deserialize {
    return __PACKAGE__->new("illegal");
}

package main;
use Storable qw( freeze thaw );
use KinoSearch::Test;
use Carp;

my $obj = BasicObj->new;
run_test_cycle( $obj, sub { ref( $_[0] ) } );

my $subclassed_obj = MyObj->new("bar");
run_test_cycle( $subclassed_obj, sub { shift->get_extra } );

my $bb = KinoSearch::Object::ByteBuf->new("foo");
run_test_cycle( $bb, sub { shift->to_perl } );

SKIP: {
    skip( "Invalid deserialization causes leaks", 1 ) if $ENV{KINO_VALGRIND};
    my $bad_obj = BadObj->new("Royale With Cheese");
    eval {
        run_test_cycle( $bad_obj, sub { ref( $_[0] ) } );
    };
    like( $@, qr/BadObj/i, "throw error with bad deserialize" );
}

sub run_test_cycle {
    my ( $orig, $transform ) = @_;
    my $class = ref($orig);

    my $frozen = freeze($orig);
    my $thawed = thaw($frozen);
    is( $transform->($thawed), $transform->($orig), "$class: freeze/thaw" );

    my $ram_file = KinoSearch::Store::RAMFile->new;
    my $outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
        or confess KinoSearch->error;
    $orig->serialize($outstream);
    $outstream->close;
    my $instream = KinoSearch::Store::InStream->open( file => $ram_file )
        or confess KinoSearch->error;
    my $deserialized = $class->deserialize($instream);

    is( $transform->($deserialized),
        $transform->($orig), "$class: call deserialize via class name" );
}
