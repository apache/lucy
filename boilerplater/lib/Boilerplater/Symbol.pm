use strict;
use warnings;

package Boilerplater::Symbol;
use Boilerplater::Parcel;
use Scalar::Util qw( blessed );
use Carp;

my %new_PARAMS = (
    parcel   => undef,
    exposure => undef,
);

sub new {
    my ( $either, %args ) = @_;

    # Acquire a Parcel.
    my $parcel = $args{parcel};
    if ( !defined $parcel ) {
        $parcel = Boilerplater::Parcel->default_parcel;
    }
    elsif ( blessed($parcel) ) {
        confess("Not a Boilerplater::Parcel")
            unless $parcel->isa('Boilerplater::Parcel');
    }
    else {
        $parcel = Boilerplater::Parcel->singleton( name => $args{parcel} );
    }

    # Create the object.
    my $self = bless { %new_PARAMS, %args, parcel => $parcel },
        ref($either) || $either;

    # Validate the symbol's exposure, then cache accessor values.
    confess("Invalid value for 'exposure': $self->{exposure}")
        unless $self->{exposure} =~ /^(?:public|parcel|private|local)$/;
    $self->{_public}  = $self->{exposure} eq 'public'  ? 1 : 0;
    $self->{_private} = $self->{exposure} eq 'private' ? 1 : 0;
    $self->{_parcel}  = $self->{exposure} eq 'parcel'  ? 1 : 0;
    $self->{_local}   = $self->{exposure} eq 'local'   ? 1 : 0;

    return $self;
}

sub set_parcel {
    my ( $self, $parcel ) = @_;
    if ( blessed($parcel) and $parcel->isa('Boilerplater::Parcel') ) {
        $self->{parcel} = $parcel;
    }
    else {
        $self->{parcel} = Boilerplater::Parcel->singleton( name => $parcel );
    }
}
sub get_parcel { shift->{parcel} }

sub get_prefix { shift->{parcel}->get_prefix; }
sub get_Prefix { shift->{parcel}->get_Prefix; }
sub get_PREFIX { shift->{parcel}->get_PREFIX; }

sub public  { shift->{_public} }
sub private { shift->{_private} }
sub parcel  { shift->{_parcel} }
sub local   { shift->{_local} }

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless $self->{parcel}->equals( $other->{parcel} );
    if ( defined $self->{exposure} ) {
        return 0 unless defined $other->{exposure};
        return 0 unless $self->{exposure} eq $other->{exposure};
    }
    else {
        return 0 if defined $other->{exposure};
    }
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Boilerplater::Symbol - Abstract base class for Boilerplater symbols.

=head1 DESCRIPTION

Boilerplater::Symbol serves as an abstract parent class for entities which may
live in the global namespace, such as classes, functions, methods, and
variables.

=head1 CONSTRUCTOR

    my $symbol = MySymbol->new(
        parcel   => $parcel,      # required
        exposure => $exposure,    # required
    );

=over

=item

B<parcel> - A Boilerplater::Parcel, or a string that can be used to
create/retrieve one.

=item

B<exposure> - The scope in which the symbol is exposed.  Must be 'public',
'parcel', 'private', or 'local'.

=back

=head1 OBJECT METHODS

=head2 get_parcel set_parcel

Accessors for C<parcel> member var.

=head2 get_prefix get_Prefix get_PREFIX

Get a string prefix, delegating to C<parcel> member var.

=head2 public parcel private local

    if    ( $sym->public ) { do_x() }
    elsif ( $sym->parcel ) { do_y() }

Indicate whether the symbol matches a given access level.

=head2 equals

    do_stuff() if $sym->equals($other_sym);

Returns true if the symbols are "equal", false otherwise.

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

