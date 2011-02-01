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

package Clownfish::Function;
use base qw( Clownfish::Symbol );
use Carp;
use Clownfish::Util qw( verify_args a_isa_b );
use Clownfish::Type;
use Clownfish::ParamList;

our %return_type;
our %param_list;
our %docucomment;
our %inline;

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
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $return_type = delete $args{return_type};
    my $param_list  = delete $args{param_list};
    my $docucomment = delete $args{docucomment};
    my $inline      = delete $args{inline} || 0;
    my $self        = $either->SUPER::new( %new_PARAMS, %args );
    $return_type{$self} = $return_type;
    $param_list{$self}  = $param_list;
    $docucomment{$self} = $docucomment;
    $inline{$self}      = $inline;

    # Validate.
    for (qw( return_type class_name param_list )) {
        my $meth_name = "get_$_";
        confess("$_ is mandatory") unless defined $self->$meth_name;
    }
    confess( "Invalid micro_sym: '" . $self->micro_sym . "'" )
        unless $self->micro_sym =~ /^[a-z0-9_]+$/;
    confess 'param_list must be a ParamList object'
        unless a_isa_b( $param_list, "Clownfish::ParamList" );
    confess 'return_type must be a Type object'
        unless a_isa_b( $return_type, "Clownfish::Type" );

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $return_type{$self};
    delete $param_list{$self};
    delete $docucomment{$self};
    delete $inline{$self};
}

sub get_return_type { $return_type{ +shift } }
sub get_param_list  { $param_list{ +shift } }
sub get_docucomment { $docucomment{ +shift } }
sub inline          { $inline{ +shift } }

sub void { shift->get_return_type->is_void }

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

=cut
