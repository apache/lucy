use strict;
use warnings;

package Clownfish::Type::Primitive;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = (
    const     => undef,
    specifier => undef,
    c_string  => undef,
);

sub new {
    my ( $either, %args ) = @_;
    my $package = ref($either) || $either;
    confess( __PACKAGE__ . " is abstract" ) if $package eq __PACKAGE__;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    return bless { %new_PARAMS, %args }, $package;
}

sub is_primitive {1}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa(__PACKAGE__);
    return 0 unless $self->{specifier} eq $other->{specifier};
    return 0 if ( $self->{const} xor $other->{const} );
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Type::Primitive - Abstract base class for primitive types.

=head1 DESCRIPTION

Clownfish::Type::Primitive serves as a common parent class for primitive
types including L<Clownfish::Type::Integer> and
L<Clownfish::Type::Float>.

=head1 METHODS

=head2 new

    my $type = MyPrimitiveType->new(
        const     => 1,       # default: undef
        specifier => 'char',  # default: undef
        c_string  => 'char',  # default: undef
    );

Abstract constructor.  See L<Clownfish::Type> for parameter definitions.

=head1 COPYRIGHT AND LICENSE

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut
