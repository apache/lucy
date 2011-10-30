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

package Clownfish::Binding::Perl::Pod;
use base qw( Clownfish::Base );
use Clownfish::Util qw( verify_args );
use Carp;

our %new_PARAMS = (
    description  => undef,
    synopsis     => undef,
    constructor  => undef,
    constructors => undef,
    methods      => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $synopsis    = $args{synopsis}    || '';
    my $description = $args{description} || '';
    my $methods     = $args{methods}     || [];
    my $self = _new( $synopsis, $description );
    for (@$methods) {
        if (ref($_)) {
            _add_method( $self, $_->{name}, $_->{pod} );
        }
        else {
            _add_method( $self, $_, undef);
        }
    }
    return $self;
}

1;

__END__

