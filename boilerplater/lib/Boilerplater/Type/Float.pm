use strict;
use warnings;

package Boilerplater::Type::Float;
use base qw( Boilerplater::Type::Primitive );
use Boilerplater::Util qw( verify_args );
use Carp;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
);

our %specifiers = (
    float  => undef,
    double => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    confess("Unknown specifier: '$args{specifier}'")
        unless exists $specifiers{ $args{specifier} };

    # Cache the C representation of this type.
    my $c_string = $args{const} ? "const $args{specifier}" : $args{specifier};

    return $either->SUPER::new( %args, c_string => $c_string );
}

sub is_floating {1}

1;

__END__

__POD__

=head1 NAME

Boilerplater::Type::Float - A primitive Type representing a floating point
number.

=head1 DESCRIPTION

Boilerplater::Type::Float represents floating point types of various widths.
Currently only two are supported:

    float
    double

=head1 METHODS

=head2 new

    my $type = Boilerplater::Type::Float->new(
        const     => 1,           # default: undef
        specifier => 'double',    # required
    );

=over

=item * B<const> - Should be true if the type is const.

=item * B<specifier> - Must match one of the supported types.

=back

=head1 COPYRIGHT AND LICENSE

    /**
     * Copyright 2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

=cut

