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

package Clownfish::Type::Arbitrary;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = (
    parcel    => undef,
    specifier => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = $either->SUPER::new(@_);

    # Validate specifier.
    confess("illegal specifier: '$self->{specifier}")
        unless $self->{specifier} =~ /^\w+$/;

    if ( $self->{specifier} =~ /^[A-Z]/ and $self->{parcel} ) {
        my $prefix = $self->{parcel}->get_prefix;
        # Add $prefix to what appear to be namespaced types.
        $self->{specifier} = $prefix . $self->{specifier}
            unless $self->{specifier} =~ /^$prefix/;
    }

    # Cache C representation.
    my $string = $self->const ? 'const ' : '';
    $string .= $self->{specifier};
    $self->set_c_string($string);

    return $self;
}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa(__PACKAGE__);
    return 0 unless $self->{specifier} eq $other->{specifier};
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Type::Arbitrary - An arbitrary type.

=head1 DESCRIPTION

The "arbitrary" type class is a hack that spares us from having to support C
types with complex declaration syntaxes -- such as unions, structs, enums, or
function pointers -- from within Clownfish itself.

The only constraint is that the C<specifier> must end in "_t".  This allows us
to create complex types in a C header file...

    typedef union { float f; int i; } floatint_t;

... pound-include the C header, then use the resulting typedef in a Clownfish
header file and have it parse as an "arbitrary" type.

    floatint_t floatint;

=head1 METHODS

=head2 new

    my $type = Clownfish::Type->new(
        specifier => 'floatint_t',    # required
        parcel    => 'Crustacean',    # default: undef
    );

=over

=item * B<specifier> - The name of the type, which must end in "_t".

=item * B<parcel> - A L<Clownfish::Parcel> or a parcel name.

=back

If C<parcel> is supplied and C<specifier> begins with a capital letter, the
Parcel's prefix will be prepended to the specifier:

    foo_t         -> foo_t                # no prefix prepending
    Lobster_foo_t -> crust_Lobster_foo_t  # prefix prepended

=cut
