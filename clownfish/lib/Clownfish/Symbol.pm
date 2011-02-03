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

package Clownfish::Symbol;
use Clownfish;
use Clownfish::Parcel;
use Clownfish::Util qw( verify_args );
use Carp;

my %new_PARAMS = (
    parcel      => undef,
    exposure    => undef,
    class_name  => undef,
    class_cnick => undef,
    micro_sym   => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    $args{parcel} = Clownfish::Parcel->acquire( $args{parcel} );
    my $class_class = ref($either) || $either;
    return $class_class->_new(
        @args{qw( parcel exposure class_name class_cnick micro_sym )} );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Symbol - Abstract base class for Clownfish symbols.

=head1 DESCRIPTION

Clownfish::Symbol serves as an abstract parent class for entities which may
live in the global namespace, such as classes, functions, methods, and
variables.

=head1 CONSTRUCTOR

    my $symbol = MySymbol->new(
        parcel      => 'Crustacean',             # default: special
        exposure    => 'parcel',                 # required
        class_name  => 'Crustacean::Lobster',    # default: undef
        class_cnick => undef,                    # default: special
        micro_sym   => 'average_lifespan',       # required
    );

=over

=item * B<parcel> - A Clownfish::Parcel, or a string that can be used to
create/retrieve one.  If not supplied, will be assigned to the default Parcel.

=item * B<exposure> - The scope in which the symbol is exposed.  Must be
'public', 'parcel', 'private', or 'local'.

=item * B<class_name> - A optional class name, consisting of one or more
components separated by "::".  Each component must start with a capital
letter, contain at least one lower-case letter, and consist entirely of the
characters [A-Za-z0-9].

=item * B<class_cnick> - The C nickname associated with the supplied class
name.  If not supplied, will be derived if possible from C<class_name> by
extracting the last class name component.

=item * B<micro_sym> - The local identifier for the symbol.

=back

=head1 OBJECT METHODS

=head2 get_parcel get_class_name get_class_cnick get_exposure micro_sym

Getters.

=head2 get_prefix get_Prefix get_PREFIX

Get a string prefix, delegating to C<parcel> member var.

=head2 public parcel private local

    if    ( $sym->public ) { do_x() }
    elsif ( $sym->parcel ) { do_y() }

Indicate whether the symbol matches a given access level.

=head2 equals

    do_stuff() if $sym->equals($other_sym);

Returns true if the symbols are "equal", false otherwise.

=head2 short_sym

    # e.g. "Lobster_average_lifespan"
    print $symbol->short_sym;

Returns the C representation for the symbol minus the parcel's prefix.

=head2 full_sym

    # e.g. "crust_Lobster_average_lifespan"
    print $symbol->full_sym;

Returns the fully qualified C representation for the symbol.

=cut

