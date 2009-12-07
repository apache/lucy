use strict;
use warnings;

package Boilerplater::Type::Integer;
use base qw( Boilerplater::Type::Primitive );
use Boilerplater::Util qw( verify_args );
use Carp;
use Config;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
);

our %specifiers = (
    bool_t => $Config{intsize},
    i8_t   => 1,
    i16_t  => 2,
    i32_t  => 4,
    i64_t  => 8,
    u8_t   => 1,
    u16_t  => 2,
    u32_t  => 4,
    u64_t  => 8,
    char   => 1,
    int    => $Config{intsize},
    short  => $Config{shortsize},
    long   => $Config{longsize},
    size_t => $Config{sizesize},
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $sizeof = $specifiers{ $args{specifier} }
        or confess("Unknown specifier: '$args{specifier}'");

    # Cache the C representation of this type.
    my $c_string = $args{const} ? 'const ' : '';
    if ( $args{specifier} =~ /^(?:[iu]\d+|bool)_t$/ ) {
        $c_string .= "chy_";
    }
    $c_string .= $args{specifier};

    my $self = $either->SUPER::new( %args, c_string => $c_string );
    $self->{sizeof} = $sizeof;
    return $self;
}

sub is_integer {1}
sub sizeof     { shift->{sizeof} }

1;

__END__

__POD__

=head1 NAME

Boilerplater::Type::Integer - A primitive Type representing an integer.

=head1 DESCRIPTION

Boilerplater::Type::Integer holds integer types of various widths and various
styles.  A few standard C integer types are supported:

    char
    short
    int
    long
    size_t

Many others are not: the types from "inttypes.h", "signed" or "unsigned"
anything, "long long", "ptrdiff_t", "off_t", etc.  

Instead, the following Charmonizer typedefs are supported:

    bool_t
    i8_t
    i16_t
    i32_t
    i64_t
    u8_t
    u16_t
    u32_t
    u64_t

=head1 METHODS

=head2 new

    my $type = Boilerplater::Type::Integer->new(
        const     => 1,       # default: undef
        specifier => 'char',  # required
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

