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

package Clownfish::Type;
use Clownfish;
use Clownfish::Parcel;
use Clownfish::Util qw( verify_args a_isa_b );
use Scalar::Util qw( blessed );
use Carp;

# Inside-out member vars.
our %array;
our %child;

our %new_PARAMS = (
    const       => undef,
    specifier   => undef,
    indirection => undef,
    parcel      => undef,
    c_string    => undef,
    void        => undef,
    object      => undef,
    primitive   => undef,
    integer     => undef,
    floating    => undef,
    string_type => undef,
    va_list     => undef,
    arbitrary   => undef,
    composite   => undef,
);

sub new {
    my ( $either, %args ) = @_;
    my $package = ref($either) || $either;
    verify_args( \%new_PARAMS, %args ) or confess $@;

    my $flags = 0;
    $flags |= CONST       if $args{const};
    $flags |= NULLABLE    if $args{nullable};
    $flags |= VOID        if $args{void};
    $flags |= OBJECT      if $args{object};
    $flags |= PRIMITIVE   if $args{primitive};
    $flags |= INTEGER     if $args{integer};
    $flags |= FLOATING    if $args{floating};
    $flags |= STRING_TYPE if $args{string_type};
    $flags |= VA_LIST     if $args{va_list};
    $flags |= ARBITRARY   if $args{arbitrary};
    $flags |= COMPOSITE   if $args{composite};

    my $parcel = $args{parcel};
    if ( defined $parcel ) {
        if ( !blessed($parcel) ) {
            $parcel = Clownfish::Parcel->singleton( name => $parcel );
        }
        confess("Not a Clownfish::Parcel")
            unless $parcel->isa('Clownfish::Parcel');
    }

    my $indirection = $args{indirection} || 0;
    my $specifier   = $args{specifier}   || '';
    my $c_string    = $args{c_string}    || '';

    return $package->_new( $flags, $parcel, $specifier, $indirection,
        $c_string );
}


our %new_composite_PARAMS = (
    child       => undef,
    indirection => undef,
    array       => undef,
    nullable    => undef,
);

sub new_composite {
    my ( $either, %args ) = @_;
    my $array    = delete $args{array};
    my $child    = delete $args{child};
    my $nullable = delete $args{nullable};
    $args{indirection} ||= 0;
    confess("Missing required param 'child'")
        unless a_isa_b( $child, "Clownfish::Type" );
    verify_args( \%new_composite_PARAMS, %args ) or confess $@;
    my $self = $either->new(
        %args,
        specifier => $child->get_specifier,
        composite => 1
    );
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
    $self->_destroy;
}

sub get_array     { $array{ +shift } }
sub _get_child    { $child{ +shift } }

sub equals {
    my ( $self, $other ) = @_;
    my $child = $self->_get_child;
    if ($child) {
        return 0 unless $other->_get_child;
        return 0 unless $child->equals( $other->_get_child );
    }
    return 0 if ( $self->get_array xor $other->get_array );
    return 0 if ( $self->get_array and $self->get_array ne $other->get_array );
    return $self->_equals($other);
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Type - A variable's type.

=head1 METHODS

=head2 new

    my $type = MyType->new(
        specifier   => 'char',    # default undef
        indirection => undef,     # default 0
        const       => 1,         # default undef
        parcel      => undef,     # default undef
        c_string    => undef,     # default undef
    );

Generic constructor.

=over

=item *

B<specifier> - The C name for the type, not including any indirection or array
subscripts.  

=item *

B<indirection> - integer indicating level of indirection. Example: the C type
"float**" has a specifier of "float" and indirection 2.

=item *

B<const> - should be true if the type is const.

=item *

B<parcel> - A Clownfish::Parcel or a parcel name.

=item *

B<c_string> - The C representation of the type.

=head2 new_composite

    my $type = Clownfish::Type->new_composite(
        child       => $char_type,    # required
        indirection => undef,         # default 0
        array       => '[]',          # default undef,
        const       => 1,             # default undef
    );

Constructor for a composite type which is made up of repetitions of a single,
uniform subtype.

=over

=item *

B<child> - The Type which the composite is comprised of.

=item *

B<indirection> - integer indicating level of indirection. Example: the C type
"float**" has indirection 2.

=item *

B<array> - A string describing an array postfix.  

=item *

B<const> - should be 1 if the type is const.

=back

=head2 equals

    do_stuff() if $type->equals($other);

Returns true if two Clownfish::Type objects are equivalent.

=head2 to_c

    # Declare variable "foo".
    print $type->to_c . " foo;\n";

Return the C representation of the type.

=head2 set_c_string

Set the C representation of the type.

=head2 get_specifier get_parcel get_indirection get_array const nullable set_specifier set_nullable

Accessors.

=head2 is_object is_primitive is_integer is_floating is_composite is_void

    do_stuff() if $type->is_object;

Shorthand for various $type->isa($package) calls.  

=over

=item * is_object: Clownfish::Type::Object

=item * is_primitive: primitive, concrete type, i.e. not an object or composite.

=item * is_integer: Clownfish::Type::Integer

=item * is_floating: Clownfish::Type::Float

=item * is_void: Clownfish::Type::Void

=item * is_composite: constructed via new_composite().

=back

=head2 is_string_type

Returns true if $type represents a Clownfish type which holds unicode
strings.

=cut

