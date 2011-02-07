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

package Clownfish::Type::Integer;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Carp;

# Inside-out member vars.
our %sizeof;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
);

our %specifiers = (
    bool_t   => undef,
    int8_t   => 1,
    int16_t  => 2,
    int32_t  => 4,
    int64_t  => 8,
    uint8_t  => 1,
    uint16_t => 2,
    uint32_t => 4,
    uint64_t => 8,
    char     => 1,
    int      => undef,
    short    => undef,
    long     => undef,
    size_t   => undef,
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    confess("Unknown specifier: '$args{specifier}'")
        unless exists $specifiers{ $args{specifier} };

    # Cache the C representation of this type.
    my $c_string = $args{const} ? 'const ' : '';
    if ( $args{specifier} eq 'bool_t' ) {
        $c_string .= "chy_";
    }
    $c_string .= $args{specifier};

    my $self = $either->SUPER::new(
        %args,
        c_string  => $c_string,
        integer   => 1,
        primitive => 1,
    );
    $sizeof{$self} = $specifiers{ $args{specifier} };
    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $sizeof{$self};
    $self->SUPER::DESTROY;
}

sub sizeof { $sizeof{ +shift } }

1;

__END__

__POD__

=head1 NAME

Clownfish::Type::Integer - A primitive Type representing an integer.

=head1 DESCRIPTION

Clownfish::Type::Integer holds integer types of various widths and various
styles.  Support is limited to a subset of the standard C integer types:

    int8_t
    int16_t
    int32_t
    int64_t
    uint8_t
    uint16_t
    uint32_t
    uint64_t
    char
    short
    int
    long
    size_t

Many others are not supported: "signed" or "unsigned" anything, "long long",
"ptrdiff_t", "off_t", etc.  

The following Charmonizer typedefs are supported:

    bool_t

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::Integer->new(
        const     => 1,       # default: undef
        specifier => 'char',  # required
    );

=over

=item * B<const> - Should be true if the type is const.

=item * B<specifier> - Must match one of the supported types.

=back

=cut

