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

package Clownfish::Type::Primitive;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
    c_string  => undef,
);

sub new {
    my ( $either, %args ) = @_;
    my $package = ref($either) || $either;
    confess( __PACKAGE__ . " is abstract" ) if $package eq __PACKAGE__;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    return $package->SUPER::new( %new_PARAMS, %args );
}

sub is_primitive {1}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa(__PACKAGE__);
    return 0 unless $self->get_specifier eq $other->get_specifier;
    return 0 if ( $self->const xor $other->const );
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Type::Primitive - Abstract base class for primitive types.

=head1 DESCRIPTION

Clownfish::Type::Primitive serves as a common parent class for primitive
types including L<Clownfish::Type::Integer> and
L<Clownfish::Type::Float>.

=head1 METHODS

=head2 new

    my $type = MyPrimitiveType->new(
        const     => 1,       # default: undef
        specifier => 'char',  # default: undef
        c_string  => 'char',  # default: undef
    );

Abstract constructor.  See L<Clownfish::Type> for parameter definitions.

=cut
