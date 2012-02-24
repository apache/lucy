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

package Clownfish::CFC::Binding::Perl;
use base qw( Clownfish::CFC::Base );

use Carp;
use Clownfish::CFC::Util qw( verify_args a_isa_b );

our %new_PARAMS = (
    parcel     => undef,
    hierarchy  => undef,
    lib_dir    => undef,
    boot_class => undef,
    header     => undef,
    footer     => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    if ( !a_isa_b( $args{parcel}, 'Clownfish::CFC::Parcel' ) ) {
        $args{parcel}
            = Clownfish::CFC::Parcel->singleton( name => $args{parcel} );
    }
    return _new(
        @args{qw( parcel hierarchy lib_dir boot_class header footer )} );
}

1;

