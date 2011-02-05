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

package Clownfish::Type::Composite;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args a_isa_b );
use Scalar::Util qw( blessed );
use Carp;

# Inside-out member vars.
our %array;
our %child;

our %new_PARAMS = (
    child       => undef,
    indirection => undef,
    array       => undef,
    nullable    => undef,
);

sub new {
    my ( $either, %args ) = @_;
    my $array    = delete $args{array};
    my $child    = delete $args{child};
    my $nullable = delete $args{nullable};
    $args{indirection} ||= 0;
    confess("Missing required param 'child'")
        unless a_isa_b( $child, "Clownfish::Type" );
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $self = $either->SUPER::new( %args, composite => 1 );
    $child{$self} = $child;
    $array{$self} = $array;
    $self->set_nullable($nullable);

    # Cache C representation.
    # NOTE: Array postfixes are NOT included.
    my $string = $child->to_c;
    for ( my $i = 0; $i < $self->get_indirection; $i++ ) {
        $string .= '*';
    }
    $self->set_c_string($string);

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $array{$self};
    delete $child{$self};
    $self->SUPER::DESTROY;
}

sub get_specifier { shift->_get_child->get_specifier }
sub get_array     { $array{ +shift } }
sub _get_child    { $child{ +shift } }

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless $self->get_indirection == $other->get_indirection;
    return 0 unless $self->_get_child->equals( $other->_get_child );
    return 0 if ( $self->get_array xor $other->get_array );
    return 0 if ( $self->get_array and $self->get_array ne $other->get_array );
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Type::Composite - A composite type, e.g. Obj**.

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::Composite->new(
        specifier   => 'char',    # required
        indirection => undef,     # default 0
        array       => '[]',      # default undef,
        const       => 1,         # default undef
    );

=over

=item *

B<specifier> - The name of the type, not including any indirection or array
subscripts.  If the type begins with a capital letter, it will be assumed to
be an object type.

=item *

B<indirection> - integer indicating level of indirection. Example: the C type
"float**" has a specifier of "float" and indirection 2.

=item *

B<array> - A string describing an array postfix.  

=item *

B<const> - should be 1 if the type is const.

=back

=head2 get_array

Accessor for the array string.

=head2 get_specifier

Overridden to return the child Type's specifier.

=cut

