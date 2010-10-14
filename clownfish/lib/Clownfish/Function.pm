use strict;
use warnings;

package Clownfish::Function;
use base qw( Clownfish::Symbol );
use Carp;
use Clownfish::Util qw( verify_args a_isa_b );
use Clownfish::Type;
use Clownfish::ParamList;

my %new_PARAMS = (
    return_type => undef,
    class_name  => undef,
    class_cnick => undef,
    param_list  => undef,
    micro_sym   => undef,
    docucomment => undef,
    parcel      => undef,
    inline      => 0,
    exposure    => 'parcel',
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = $either->SUPER::new( %new_PARAMS, @_ );

    # Validate.
    for (qw( return_type class_name param_list )) {
        confess("$_ is mandatory") unless defined $self->{$_};
    }
    confess("Invalid micro_sym: '$self->{micro_sym}'")
        unless $self->{micro_sym} =~ /^[a-z0-9_]+$/;
    confess 'param_list must be a ParamList object'
        unless a_isa_b( $self->{param_list}, "Clownfish::ParamList" );
    confess 'return_type must be a Type object'
        unless a_isa_b( $self->{return_type}, "Clownfish::Type" );

    return $self;
}

sub get_return_type { shift->{return_type} }
sub get_param_list  { shift->{param_list} }
sub get_docucomment { shift->{docucomment} }
sub inline          { shift->{inline} }

sub void { shift->{return_type}->is_void }

sub full_func_sym  { shift->SUPER::full_sym }
sub short_func_sym { shift->SUPER::short_sym }

1;

__END__

__POD__

=head1 NAME

Clownfish::Function - Metadata describing a function.

=head1 METHODS

=head2 new

    my $type = Clownfish::Function->new(
        class_name  => 'Crustacean::Lobster::LobsterClaw',  # required
        class_cnick => 'LobClaw',                           # default: special
        return_type => $int_type,                           # required
        param_list  => $param_list,                         # required
        micro_sym   => 'compare',                           # required
        docucomment => $docucomment,                        # default: undef
        parcel      => 'Crustacean',                        # default: special
        exposure    => 'public',                            # default: parcel
        inline      => 1,                                   # default: false
    );

=over

=item * B<class_name> - The full name of the class in whose namespace the
function resides.

=item * B<class_cnick> - The C nickname for the class. 

=item * B<return_type> - A L<Clownfish::Type> representing the function's
return type.

=item * B<param_list> - A L<Clownfish::ParamList> representing the
function's argument list.

=item * B<micro_sym> - The lower case name of the function, without any
namespacing prefixes.

=item * B<docucomment> - A L<Clownfish::DocuComment> describing the
function.

=item * B<parcel> - A L<Clownfish::Parcel> or a parcel name.

=item * B<exposure> - The function's exposure (see L<Clownfish::Symbol>).

=item * B<inline> - Should be true if the function should be inlined by the
compiler.

=back

=head2 get_return_type get_param_list get_docucomment inline 

Accessors.

=head2 void

Returns true if the function has a void return type, false otherwise.

=head2 full_func_sym

A synonym for full_sym().

=head2 short_func_sym

A synonym for short_sym().

=head1 COPYRIGHT AND LICENSE

Copyright 2006-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut
