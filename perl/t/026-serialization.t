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

use Test::More tests => 5;

package BasicObj;
use base qw( Lucy::Search::Query );

package MyObj;
use base qw( Lucy::Search::Query );

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
use Lucy::Test;
use Carp;

my $obj = BasicObj->new;
run_test_cycle( $obj, sub { ref( $_[0] ) } );

my $subclassed_obj = MyObj->new("bar");
run_test_cycle( $subclassed_obj, sub { shift->get_extra } );

SKIP: {
    skip( "Invalid deserialization causes leaks", 1 ) if $ENV{LUCY_VALGRIND};
    my $bad_obj = BadObj->new("Royale With Cheese");
    eval {
        run_test_cycle( $bad_obj, sub { ref( $_[0] ) } );
    };
    like( $@, qr/BadObj/i, "throw error with bad deserialize" );
}

sub run_test_cycle {
    my ( $orig, $transform ) = @_;
    my $class = ref($orig);
    my $cf_class = $orig->get_class;

    my $frozen = freeze($orig);
    my $thawed = thaw($frozen);
    is( $transform->($thawed), $transform->($orig), "$class: freeze/thaw" );

    return unless $class->can("serialize");

    my $ram_file = Lucy::Store::RAMFile->new;
    my $outstream = Lucy::Store::OutStream->open( file => $ram_file )
        or confess Clownfish->error;
    $orig->serialize($outstream);
    $outstream->close;
    my $instream = Lucy::Store::InStream->open( file => $ram_file )
        or confess Clownfish->error;
    my $deserialized = $cf_class->make_obj->deserialize($instream);

    is( $transform->($deserialized),
        $transform->($orig), "$class: call deserialize via class name" );
}
