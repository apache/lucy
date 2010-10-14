use strict;
use warnings;

package Clownfish::Type::Integer;
use base qw( Clownfish::Type::Primitive );
use Clownfish::Util qw( verify_args );
use Carp;
use Config;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
);

our %specifiers = (
    bool_t   => $Config{intsize},
    int8_t   => 1,
    int16_t  => 2,
    int32_t  => 4,
    int64_t  => 8,
    uint8_t  => 1,
    uint16_t => 2,
    uint32_t => 4,
    uint64_t => 8,
    char     => 1,
    int      => $Config{intsize},
    short    => $Config{shortsize},
    long     => $Config{longsize},
    size_t   => $Config{sizesize},
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $sizeof = $specifiers{ $args{specifier} }
        or confess("Unknown specifier: '$args{specifier}'");

    # Cache the C representation of this type.
    my $c_string = $args{const} ? 'const ' : '';
    if ( $args{specifier} eq 'bool_t' ) {
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

=head1 COPYRIGHT AND LICENSE

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut

