use strict;
use warnings;

package Clownfish::Type::VAList;
use base qw( Clownfish::Type );
use Clownfish::Util qw( verify_args );
use Scalar::Util qw( blessed );
use Carp;

our %new_PARAMS = ( specifier => 'va_list' );

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    return $either->SUPER::new(
        specifier => 'va_list',
        c_string  => 'va_list'
    );
}

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

Clownfish::Type::VAList - A Type to support C's va_list.

=head1 DESCRIPTION

Clownfish::Type::VAList represents the C va_list type, from stdarg.h.

=head1 METHODS

=head2 new

    my $type = Clownfish::Type::VAList->new(
        specifier => 'va_list',    # default: va_list
    );

=over

=item * B<specifier>.  Must be "va_list" if supplied.

=back

=cut

