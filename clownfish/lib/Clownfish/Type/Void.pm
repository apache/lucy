use strict;
use warnings;

package Clownfish::Type::Void;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = ( 
    const     => undef,
    specifier => 'void', 
);

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $c_string = $args{const} ? "const void" : "void";
    return $either->SUPER::new(
        %new_PARAMS,
        %args,
        specifier => 'void',
        c_string  => $c_string
    );
}

sub is_void {1}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless blessed($other);
    return 0 unless $other->isa(__PACKAGE__);
    return 1;
}

1;

__END__

=head1 NAME

Clownfish::Type::Void - The void Type.

=head1 DESCRIPTION

Clownfish::Type::Void is used to represent a void return type.  It is also
used in conjuction with with L<Clownfish::Type::Composite> to support the
C<void*> opaque pointer type.

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::Void->new(
        specifier => 'void',    # default: void
        const     => 1,         # default: undef
    );

=over

=item * B<specifier> - Must be "void" if supplied.

=item * B<const> - Should be true if the type is const.  (Useful in the
context of C<const void*>).

=back

=head1 COPYRIGHT AND LICENSE

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut

