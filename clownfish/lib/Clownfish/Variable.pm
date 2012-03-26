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

package Clownfish::Variable;
use base qw( Clownfish::Symbol );
use Clownfish;
use Clownfish::Util qw( verify_args );
use Carp;

our %new_PARAMS = (
    type        => undef,
    micro_sym   => undef,
    parcel      => undef,
    exposure    => 'local',
    class_name  => undef,
    class_cnick => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    $args{exposure} ||= 'local';
    $args{parcel} = Clownfish::Parcel->acquire( $args{parcel} );
    my $package = ref($either) || $either;
    return $package->_new(
        @args{qw( parcel exposure class_name class_cnick micro_sym type )} );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Variable - A Clownfish variable.

=head1 DESCRIPTION

A variable, having a L<Type|Clownfish::Type>, a micro_sym (i.e. name), an
exposure, and optionally, a location in the global namespace hierarchy.

Variable objects which exist only within a local scope, e.g. those within
parameter lists, do not need to know about class.  In contrast, inert class
vars, for example, need to know class information so that they can declare
themselves properly.

=head1 METHODS

=head2 new

    my $var = Clownfish::Variable->new(
        parcel      => 'Crustacean',
        type        => $int32_t_type,            # required
        micro_sym   => 'average_lifespan',       # required
        exposure    => 'parcel',                 # default: 'local'
        class_name  => "Crustacean::Lobster",    # default: undef
        class_cnick => "Lobster",                # default: undef
    );

=over

=item * B<type> - A L<Clownfish::Type>.

=item * B<micro_sym> - The variable's name, without any namespacing prefixes.

=item * B<exposure> - See L<Clownfish::Symbol>.

=item * B<class_name> - See L<Clownfish::Symbol>.

=item * B<class_cnick> - See L<Clownfish::Symbol>.

=back

=head2 local_c

    # e.g. "int32_t average_lifespan"
    print $variable->local_c;

Returns a string with the Variable's C type and its C<micro_sym>.

=head2 global_c

    # e.g. "int32_t crust_Lobster_average_lifespan"
    print $variable->global_c;

Returns a string with the Variable's C type and its fully qualified name
within the global namespace.

=head2 local_declaration

    # e.g. "int32_t average_lifespan;"
    print $variable->local_declaration;

Returns C code appropriate for declaring the variable in a local scope, such
as within a struct definition, or as an automatic variable within a C
function.  

=cut
