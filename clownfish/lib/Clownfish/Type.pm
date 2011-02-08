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
our %incremented;
our %decremented;
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

our %new_object_PARAMS = (
    const       => undef,
    specifier   => undef,
    indirection => 1,
    parcel      => undef,
    incremented => 0,
    decremented => 0,
    nullable    => 0,
);

sub new_object {
    my ( $either, %args ) = @_;
    verify_args( \%new_object_PARAMS, %args ) or confess $@;
    my $incremented = delete $args{incremented} || 0;
    my $decremented = delete $args{decremented} || 0;
    my $nullable    = delete $args{nullable}    || 0;
    $args{indirection} = 1 unless defined $args{indirection};
    my $indirection = $args{indirection};
    $args{parcel} ||= Clownfish::Parcel->default_parcel;
    confess("Missing required param 'specifier'")
        unless defined $args{specifier};

    # Derive boolean indicating whether this type is a string type.
    my $is_string_type = $args{specifier} =~ /CharBuf/ ? 1 : 0;

    my $self = $either->new(
        %args,
        object      => 1,
        string_type => $is_string_type,
    );
    $incremented{$self} = $incremented;
    $decremented{$self} = $decremented;
    $self->set_nullable($nullable);
    my $prefix    = $self->get_parcel->get_prefix;
    my $specifier = $self->get_specifier;

    # Validate params.
    confess("Indirection must be 1") unless $indirection == 1;
    confess("Can't be both incremented and decremented")
        if ( $incremented && $decremented );
    confess("Illegal specifier: '$specifier'")
        unless $specifier
            =~ /^(?:$prefix)?[A-Z][A-Za-z0-9]*[a-z]+[A-Za-z0-9]*(?!\w)/;

    # Add $prefix if necessary.
    if ( $specifier !~ /^$prefix/ ) {
        $specifier = $prefix . $specifier;
        $self->set_specifier($specifier);
    }

    # Cache C representation.
    my $string = $self->const ? 'const ' : '';
    $string .= "$specifier*";
    $self->set_c_string($string);

    return $self;
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

our %new_void_PARAMS = (
    const     => undef,
    specifier => 'void',
);

sub new_void {
    my ( $either, %args ) = @_;
    verify_args( \%new_void_PARAMS, %args ) or confess $@;
    my $c_string = $args{const} ? "const void" : "void";
    return $either->new(
        %new_PARAMS,
        %args,
        specifier => 'void',
        c_string  => $c_string,
        void      => 1,
    );
}

our %new_va_list_PARAMS = ( specifier => 'va_list' );

sub new_va_list {
    my ( $either, %args ) = @_;
    verify_args( \%new_va_list_PARAMS, %args ) or confess $@;
    return $either->new(
        specifier => 'va_list',
        c_string  => 'va_list',
        va_list   => 1,
    );
}

our %new_arbitrary_PARAMS = (
    parcel    => undef,
    specifier => undef,
);

sub new_arbitrary {
    my $either = shift;
    verify_args( \%new_arbitrary_PARAMS, @_ ) or confess $@;
    my $self = $either->new( @_, arbitrary => 1 );

    # Validate specifier.
    my $specifier = $self->get_specifier;
    my $parcel    = $self->get_parcel;
    confess("illegal specifier: '$specifier'")
        unless $specifier =~ /^\w+$/;
    if ( $specifier =~ /^[A-Z]/ and $parcel ) {
        my $prefix = $parcel->get_prefix;
        # Add $prefix to what appear to be namespaced types.
        if ( $specifier !~ /^$prefix/ ) {
            $specifier = $prefix . $specifier;
            $self->set_specifier($specifier);
        }
    }

    # Cache C representation.
    my $string = $self->const ? 'const ' : '';
    $string .= $specifier;
    $self->set_c_string($string);

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $array{$self};
    delete $child{$self};
    delete $incremented{$self};
    delete $decremented{$self};
    $self->_destroy;
}

sub get_array     { $array{ +shift } }
sub _get_child    { $child{ +shift } }
sub incremented   { $incremented{ +shift } }
sub decremented   { $decremented{ +shift } }

sub similar {
    my ( $self, $other ) = @_;
    confess("Not an object type") unless $self->is_object;
    for (qw( is_object const incremented decremented nullable )) {
        return 0 if ( $self->$_ xor $other->$_ );
    }
    return 1;
}

sub equals {
    my ( $self, $other ) = @_;
    my $child = $self->_get_child;
    if ($child) {
        return 0 unless $other->_get_child;
        return 0 unless $child->equals( $other->_get_child );
    }
    return 0 if ( $self->incremented xor $other->incremented );
    return 0 if ( $self->decremented xor $other->decremented );
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

=back

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

=head2 new_object

    my $type = Clownfish::Type->new_object(
        specifier   => "Lobster",       # required
        parcel      => "Crustacean",    # default: the default Parcel.
        const       => undef,           # default undef
        indirection => 1,               # default 1
        incremented => 1,               # default 0
        decremented => 0,               # default 0
        nullable    => 1,               # default 0
    );

Create a Type representing an object.  The Type's C<specifier> must match the
last component of the class name -- i.e. for the class "Crustacean::Lobster"
it must be "Lobster".

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

The Parcel's prefix will be prepended to the specifier by new_object().

=head2 new_void

    my $type = Clownfish::Type->new_void(
        specifier => 'void',    # default: void
        const     => 1,         # default: undef
    );

Return a Type representing a the 'void' keyword in C.  It can be used either
for a void return type, or in conjuction with with new_composite() to support
the C<void*> opaque pointer type.

=over

=item * B<specifier> - Must be "void" if supplied.

=item * B<const> - Should be true if the type is const.  (Useful in the
context of C<const void*>).

=back

=head2 new_va_list

    my $type = Clownfish::Type->new_va_list(
        specifier => 'va_list',    # default: va_list
    );

Create a Type representing C's va_list, from stdarg.h.

=over

=item * B<specifier>.  Must be "va_list" if supplied.

=back

=head2 new_arbitrary

    my $type = Clownfish::Type->new_arbitrary(
        specifier => 'floatint_t',    # required
        parcel    => 'Crustacean',    # default: undef
    );

"Arbitrary" types are a hack that spares us from having to support C types
with complex declaration syntaxes -- such as unions, structs, enums, or
function pointers -- from within Clownfish itself.

The only constraint is that the C<specifier> must end in "_t".  This allows us
to create complex types in a C header file...

    typedef union { float f; int i; } floatint_t;

... pound-include the C header, then use the resulting typedef in a Clownfish
header file and have it parse as an "arbitrary" type.

    floatint_t floatint;

=over

=item * B<specifier> - The name of the type, which must end in "_t".

=item * B<parcel> - A L<Clownfish::Parcel> or a parcel name.

=back

If C<parcel> is supplied and C<specifier> begins with a capital letter, the
Parcel's prefix will be prepended to the specifier:

    foo_t         -> foo_t                # no prefix prepending
    Lobster_foo_t -> crust_Lobster_foo_t  # prefix prepended

=cut

=head2 equals

    do_stuff() if $type->equals($other);

Returns true if two Clownfish::Type objects are equivalent.

=head2 similar

    do_stuff() if $type->similar($other_type);

Weak checking of type which allows for covariant return types.  Calling this
method on anything other than an object type is an error.

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

Identify the flavor of Type, which is determined by the constructor which was
used to create it.

=over

=item * is_object: Clownfish::Type->new_object

=item * is_primitive: Either Clownfish::Type::Integer->new or
Clownfish::Type::Float->new

=item * is_integer: Clownfish::Type::Integer->new

=item * is_floating: Clownfish::Type::Float->new

=item * is_void: Clownfish::Type->new_void

=item * is_composite: Clownfish::Type->new_composite

=back

=head2 is_string_type

Returns true if $type represents a Clownfish type which holds unicode
strings.

=head2 incremented

Returns true if the Type is incremented.  Only applicable to object Types.

=head2 decremented

Returns true if the Type is decremented.  Only applicable to object Types.

=cut

