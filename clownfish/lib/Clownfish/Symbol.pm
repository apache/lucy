use strict;
use warnings;

package Clownfish::Symbol;
use Clownfish::Parcel;
use Clownfish::Util qw( a_isa_b );
use Scalar::Util qw( blessed );
use Carp;

my %new_PARAMS = (
    parcel      => undef,
    exposure    => undef,
    class_name  => undef,
    class_cnick => undef,
    micro_sym   => undef,
);

my $struct_regex     = qr/[A-Z]+[A-Z0-9]*[a-z]+[A-Za-z0-9]*/;
my $class_name_regex = qr/^$struct_regex(::$struct_regex)*$/;

sub new {
    my ( $either, %args ) = @_;

    # Acquire a Parcel.
    my $parcel = $args{parcel};
    if ( !defined $parcel ) {
        $parcel = Clownfish::Parcel->default_parcel;
    }
    elsif ( blessed($parcel) ) {
        confess("Not a Clownfish::Parcel")
            unless $parcel->isa('Clownfish::Parcel');
    }
    else {
        $parcel = Clownfish::Parcel->singleton( name => $args{parcel} );
    }

    # Create the object.
    my $self = bless { %new_PARAMS, %args, parcel => $parcel },
        ref($either) || $either;

    # Validate micro_sym.
    confess "micro_sym is required" unless $self->{micro_sym};
    confess("Invalid micro_sym: '$self->{micro_sym}'")
        unless $self->{micro_sym} =~ /^[A-Za-z_][A-Za-z0-9_]*$/;

    # Validate exposure.
    confess("Invalid value for 'exposure': $self->{exposure}")
        unless $self->{exposure} =~ /^(?:public|parcel|private|local)$/;

    # Validate class name, validate or derive class_cnick.
    if ( defined $self->{class_name} ) {
        confess("Invalid class name: $self->{class_name}")
            unless $self->{class_name} =~ $class_name_regex;
        if ( !defined $self->{class_cnick} ) {
            $self->{class_name} =~ /(\w+)$/;
            $self->{class_cnick} = $1;
        }
        confess("Invalid class_cnick: $self->{class_cnick}")
            unless $self->{class_cnick} =~ /^[A-Z]+[A-Za-z0-9]*$/;
    }
    elsif ( defined $self->{class_cnick} ) {
        # Sanity check class_cnick without class_name.
        confess("Can't supply class_cnick without class_name");
    }

    return $self;
}

sub get_parcel      { shift->{parcel} }
sub get_class_name  { shift->{class_name} }
sub get_class_cnick { shift->{class_cnick} }
sub micro_sym       { shift->{micro_sym} }

sub get_prefix { shift->{parcel}->get_prefix; }
sub get_Prefix { shift->{parcel}->get_Prefix; }
sub get_PREFIX { shift->{parcel}->get_PREFIX; }

sub public  { shift->{exposure} eq 'public' }
sub private { shift->{exposure} eq 'private' }
sub parcel  { shift->{exposure} eq 'parcel' }
sub local   { shift->{exposure} eq 'local' }

sub full_sym {
    my $self   = shift;
    my $prefix = $self->get_prefix;
    return "$prefix$self->{class_cnick}_$self->{micro_sym}";
}

sub short_sym {
    my $self = shift;
    return "$self->{class_cnick}_$self->{micro_sym}";
}

sub equals {
    my ( $self, $other ) = @_;
    return 0 unless a_isa_b( $other, __PACKAGE__ );
    return 0 unless $self->{micro_sym} eq $other->{micro_sym};
    return 0 unless $self->{parcel}->equals( $other->{parcel} );
    if ( defined $self->{exposure} ) {
        return 0 unless defined $other->{exposure};
        return 0 unless $self->{exposure} eq $other->{exposure};
    }
    else {
        return 0 if defined $other->{exposure};
    }
    if ( defined $self->{class_name} ) {
        return 0 unless defined $other->{class_name};
        return 0 unless $self->{class_name} eq $other->{class_name};
    }
    else {
        return 0 if defined $other->{class_name};
    }
    return 1;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Symbol - Abstract base class for Clownfish symbols.

=head1 DESCRIPTION

Clownfish::Symbol serves as an abstract parent class for entities which may
live in the global namespace, such as classes, functions, methods, and
variables.

=head1 CONSTRUCTOR

    my $symbol = MySymbol->new(
        parcel      => $parcel,                             # default: special 
        exposure    => $exposure,                           # required
        class_name  => "Crustacean::Lobster::LobsterClaw",  # default: undef
        class_cnick => "LobClaw",                           # default: special
        micro_sym   => "rubber_band"                        # required
    );

=over

=item * B<parcel> - A Clownfish::Parcel, or a string that can be used to
create/retrieve one.  If not supplied, will be assigned to the default Parcel.

=item * B<exposure> - The scope in which the symbol is exposed.  Must be
'public', 'parcel', 'private', or 'local'.

=item * B<class_name> - A optional class name, consisting of one or more
components separated by "::".  Each component must start with a capital
letter, contain at least one lower-case letter, and consist entirely of the
characters [A-Za-z0-9].

=item * B<class_cnick> - The C nickname associated with the supplied class
name.  If not supplied, will be derived if possible from C<class_name> by
extracting the last class name component.

=item * B<micro_sym> - The local identifier for the symbol.

=back

=head1 OBJECT METHODS

=head2 get_parcel get_class_name get_class_cnick micro_sym

Getters.

=head2 get_prefix get_Prefix get_PREFIX

Get a string prefix, delegating to C<parcel> member var.

=head2 public parcel private local

    if    ( $sym->public ) { do_x() }
    elsif ( $sym->parcel ) { do_y() }

Indicate whether the symbol matches a given access level.

=head2 equals

    do_stuff() if $sym->equals($other_sym);

Returns true if the symbols are "equal", false otherwise.

=head2 short_sym

    # e.g. "LobClaw_rubber_band"
    print $symbol->short_sym;

Returns the C representation for the symbol minus the parcel's prefix.

=head2 full_sym

    # e.g. "crust_LobClaw_rubber_band"
    print $symbol->full_sym;

Returns the fully qualified C representation for the symbol.

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

