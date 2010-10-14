use strict;
use warnings;

package Clownfish::CBlock;
use Clownfish::Util qw( verify_args );
use Carp;

our %new_PARAMS = ( contents => undef, );

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_ }, ref($either) || $either;
    confess("Missing required param 'contents'")
        unless defined $self->{contents};
    return $self;
}

# Accessors.
sub get_contents { shift->{contents} }

1;

__END__

__POD__

=head1 NAME

Clownfish::CBlock - A block of embedded C code.

=head1 DESCRIPTION

CBlock exists to support embedding literal C code within Clownfish header
files:

    class Crustacean::Lobster {
        /* ... */

        /** Give a lobstery greeting.
         */
        inert inline void
        say_hello(Lobster *self);
    }

    __C__
    #include <stdio.h>
    static CHY_INLINE void
    crust_Lobster_say_hello(crust_Lobster *self)
    {
        printf("Prepare to die, human scum.\n");
    }
    __END_C__

=head1 METHODS

=head2 new

    my $c_block = Clownfish::CBlock->new(
        contents => $text,
    );

=over

=item * B<contents> - Raw C code.

=back

=head2 get_contents

Accessor.

=head1 COPYRIGHT AND LICENSE

Copyright 2009-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut

