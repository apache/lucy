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
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

# Inside-out member vars.

our %new_PARAMS = (
    const       => undef,
    specifier   => undef,
    indirection => undef,
    parcel      => undef,
    c_string    => undef,
    void        => undef,
);

sub new {
    my ( $either, %args ) = @_;
    my $package = ref($either) || $either;
    confess( __PACKAGE__ . "is an abstract class" )
        if $package eq __PACKAGE__;
    verify_args( \%new_PARAMS, %args ) or confess $@;

    my $flags = 0;
    $flags |= CONST    if $args{const};
    $flags |= NULLABLE if $args{nullable};
    $flags |= VOID     if $args{void};

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

sub is_object      {0}
sub is_primitive   {0}
sub is_integer     {0}
sub is_floating    {0}
sub is_composite   {0}
sub is_string_type {0}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa(__PACKAGE__);
    return 1;
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

Abstract constructor.

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

=head2 equals

    do_stuff() if $type->equals($other);

Returns true if two Clownfish::Type objects are equivalent.  The default
implementation merely checks that C<$other> is a Clownfish::Type object, so
it should be overridden in all subclasses.

=head2 to_c

    # Declare variable "foo".
    print $type->to_c . " foo;\n";

Return the C representation of the type.

=head2 set_c_string

Set the C representation of the type.

=head2 get_specifier get_parcel get_indirection const nullable set_specifier set_nullable

Accessors.

=head2 is_object is_primitive is_integer is_floating is_composite is_void

    do_stuff() if $type->is_object;

Shorthand for various $type->isa($package) calls.  

=over

=item * is_object: Clownfish::Type::Object

=item * is_primitive: Clownfish::Type::Primitive

=item * is_integer: Clownfish::Type::Integer

=item * is_floating: Clownfish::Type::Float

=item * is_void: Clownfish::Type::Void

=item * is_composite: Clownfish::Type::Composite

=back

=head2 is_string_type

Returns true if $type represents a Clownfish type which holds unicode
strings.

=cut

