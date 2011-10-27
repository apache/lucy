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

package Clownfish::Parcel;
use base qw( Exporter );
use Clownfish;
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

END {
    __PACKAGE__->reap_singletons();
}

our %singleton_PARAMS = (
    name  => undef,
    cnick => undef,
);

sub singleton {
    my ( $either, %args ) = @_;
    verify_args( \%singleton_PARAMS, %args ) or confess $@;
    my $package = ref($either) || $either;
    return $package->_singleton( @args{qw( name cnick )} );
}

sub acquire {
    my ( undef, $thing ) = @_;
    if ( !defined $thing ) {
        return Clownfish::Parcel->default_parcel;
    }
    elsif ( blessed($thing) ) {
        confess("Not a Clownfish::Parcel")
            unless $thing->isa('Clownfish::Parcel');
        return $thing;
    }
    else {
        return Clownfish::Parcel->singleton( name => $thing );
    }
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Parcel - Collection of code.

=head1 DESCRIPTION

A Parcel is a cohesive collection of code, which could, in theory, be
published as as a single entity.

Clownfish supports two-tier manual namespacing, using a prefix, an optional
class nickname, and the local symbol:

  prefix_ClassNick_local_symbol
  
Clownfish::Parcel supports the first tier, specifying initial prefixes.
These prefixes come in three capitalization variants: prefix_, Prefix_, and
PREFIX_.

=head1 CLASS METHODS

=head2 singleton 

    Clownfish::Parcel->singleton(
        name  => 'Crustacean',
        cnick => 'Crust',
    );

Add a Parcel singleton to a global registry.  May be called multiple times,
but only with compatible arguments.

=over

=item *

B<name> - The name of the parcel.

=item *

B<cnick> - The C nickname for the parcel, which will be used as a prefix for
generated global symbols.  Must be mixed case and start with a capital letter.
Defaults to C<name>.

=back

=head2 default_parcel

   $parcel ||= Clownfish::Parcel->default_parcel;

Return the singleton for default parcel, which has no prefix.

=head1 OBJECT METHODS

=head2 get_prefix get_Prefix get_PREFIX

Return one of the three capitalization variants for the parcel's prefix.

=head2 acquire

    $parcel = Clownfish::Parcel->aquire($parcel_name_or_parcel_object);

Aquire a parcel one way or another.  If the supplied argument is a Parcel,
return it.  If it's not defined, return the default Parcel.  If it's a name,
invoke singleton().

=head2 get_name get_cnick

Accessors.

=cut

