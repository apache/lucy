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
use Clownfish::Util qw( verify_args );
use Carp;

our %parcels;

our %singleton_PARAMS = (
    name  => undef,
    cnick => undef,
);

# Create the default parcel.
our $default_parcel = __PACKAGE__->singleton(
    name  => 'DEFAULT',
    cnick => '',
);

sub default_parcel {$default_parcel}

sub singleton {
    my ( $either, %args ) = @_;
    verify_args( \%singleton_PARAMS, %args ) or confess $@;
    my ( $name, $cnick ) = @args{qw( name cnick )};

    # Return the default parcel for either a blank name or an undefined name.
    return $default_parcel unless $name;

    # Return an existing singleton if the parcel has already been registered.
    my $existing = $parcels{$name};
    if ($existing) {
        if ( $cnick and $cnick ne $existing->get_cnick ) {
            confess(  "cnick '$cnick' for parcel '$name' conflicts with '"
                    . $existing->get_cnick
                    . "'" );
        }
        return $existing;
    }

    # Register new parcel.  Default cnick to name.
    my $self = bless { %singleton_PARAMS, %args, }, ref($either) || $either;
    defined $self->get_cnick or $self->{cnick} = $self->get_name;
    $parcels{$name} = $self;

    # Pre-generate prefixes.
    $self->{Prefix} = length $self->get_cnick ? $self->get_cnick . '_' : "";
    $self->{prefix} = lc( $self->get_Prefix );
    $self->{PREFIX} = uc( $self->get_Prefix );

    return $self;
}

# Accessors.
sub get_prefix { shift->{prefix} }
sub get_Prefix { shift->{Prefix} }
sub get_PREFIX { shift->{PREFIX} }
sub get_name   { shift->{name} }
sub get_cnick  { shift->{cnick} }

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless $self->get_name  eq $other->get_name;
    return 0 unless $self->get_cnick eq $other->get_cnick;
    return 1;
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

=head2 get_name get_cnick

Accessors.

=cut

