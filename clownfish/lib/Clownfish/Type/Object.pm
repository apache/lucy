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

package Clownfish::Type::Object;
use base qw( Clownfish::Type );
use Clownfish::Parcel;
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = (
    const       => undef,
    specifier   => undef,
    indirection => 1,
    parcel      => undef,
    incremented => 0,
    decremented => 0,
    nullable    => 0,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $incremented = delete $args{incremented} || 0;
    my $decremented = delete $args{decremented} || 0;
    my $nullable    = delete $args{nullable}    || 0;
    my $indirection = delete $args{indirection};
    $indirection = 1 unless defined $indirection;
    my $self = $either->SUPER::new(%args);
    $self->{incremented} = $incremented;
    $self->{decremented} = $decremented;
    $self->{indirection} = $indirection;
    $self->{nullable}    = $nullable;
    $self->{parcel} ||= Clownfish::Parcel->default_parcel;
    my $prefix = $self->{parcel}->get_prefix;

    # Validate params.
    confess("Indirection must be 1") unless $self->{indirection} == 1;
    confess("Can't be both incremented and decremented")
        if ( $incremented && $decremented );
    confess("Missing required param 'specifier'")
        unless defined $self->{specifier};
    confess("Illegal specifier: '$self->{specifier}")
        unless $self->{specifier}
            =~ /^(?:$prefix)?[A-Z][A-Za-z0-9]*[a-z]+[A-Za-z0-9]*(?!\w)/;

    # Add $prefix if necessary.
    $self->{specifier} = $prefix . $self->{specifier}
        unless $self->{specifier} =~ /^$prefix/;

    # Cache C representation.
    my $string = $self->const ? 'const ' : '';
    $string .= "$self->{specifier}*";
    $self->set_c_string($string);

    # Cache boolean indicating whether this type is a string type.
    $self->{is_string_type} = $self->{specifier} =~ /CharBuf/ ? 1 : 0;

    return $self;
}

sub is_object      {1}
sub incremented    { shift->{incremented} }
sub decremented    { shift->{decremented} }
sub is_string_type { shift->{is_string_type} }

sub set_nullable { $_[0]->{nullable} = $_[1] }

sub similar {
    my ( $self, $other ) = @_;
    for (qw( const incremented decremented nullable )) {
        return 0 if ( $self->{$_} xor $other->{$_} );
    }
    return 1;
}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless $self->similar($other);
    return 0 unless $self->{specifier} eq $other->{specifier};
    return 1;
}

1;

__END__

=head1 NAME

Clownfish::Type::Clownfish - An object Type.

=head1 DESCRIPTION

Clownfish::Type::Object supports object types for all classes.  The type's 
C<specifier> must match the last component of the class name -- i.e. for the
class "Crustacean::Lobster" it must be "Lobster".

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::Object->new(
        specifier   => "Lobster",       # required
        parcel      => "Crustacean",    # default: the default Parcel.
        const       => undef,           # default undef
        indirection => 1,               # default 1
        incremented => 1,               # default 0
        decremented => 0,               # default 0
        nullable    => 1,               # default 0
    );

=over

=item * B<specifier> - Required.  Must follow the rules for
L<Clownfish::Class> class name components.

=item * B<parcel> - A L<Clownfish::Parcel> or a parcel name.

=item * B<const> - Should be true if the Type is const.  Note that this refers
to the object itself and not the pointer.

=item * B<indirection> - Level of indirection.  Must be 1 if supplied.

=item * B<incremented> - Indicate whether the caller must take responsibility
for an added refcount.

=item * B<decremented> - Indicate whether the caller must account for
for a refcount decrement.

=item * B<nullable> - Indicate whether the object specified by this type may
be NULL.

=back

The Parcel's prefix will be prepended to the specifier by new().

=head2 incremented

Returns true if the Type is incremented.

=head2 decremented

Returns true if the Type is decremented.

=head2 similar

    do_stuff() if $type->similar($other_type);

Weak checking of type which allows for covariant return types.

=cut

