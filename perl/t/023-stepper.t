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

package MyStepper;
use base qw( Lucy::Util::Stepper );

our %number;

sub new {
    my $self = shift->SUPER::new(@_);
    $number{$$self} = 0;
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $number{$$self};
    $self->SUPER::DESTROY;
}

sub get_number { $number{ ${ +shift } } }

sub read_record {
    my ( $self, $instream ) = @_;
    $number{$$self} += $instream->read_c32;
}

package main;
use Test::More tests => 1;
use Lucy::Test;

my $folder = Lucy::Store::RAMFolder->new;
my $outstream = $folder->open_out("foo") or die Lucy->error;
$outstream->write_c32(10) for 1 .. 5;
$outstream->close;
my $instream = $folder->open_in("foo") or die Lucy->error;
my $stepper = MyStepper->new;

my @got;
while ( $instream->tell < $instream->length ) {
    $stepper->read_record($instream);
    push @got, $stepper->get_number;
}
is_deeply( \@got, [ 10, 20, 30, 40, 50 ], 'Read_Record' );

