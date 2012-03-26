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

package Clownfish::Binding::Perl::Subroutine;
use Carp;
use Scalar::Util qw( blessed );
use Clownfish::Class;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Variable;
use Clownfish::ParamList;
use Clownfish::Util qw( verify_args );

our %new_PARAMS = (
    param_list         => undef,
    alias              => undef,
    class_name         => undef,
    retval_type        => undef,
    use_labeled_params => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_, }, ref($either) || $either;
    for (qw( param_list class_name alias retval_type )) {
        confess("$_ is required") unless defined $self->{$_};
    }
    return $self;
}

sub get_class_name     { shift->{class_name} }
sub use_labeled_params { shift->{use_labeled_params} }

sub perl_name {
    my $self = shift;
    return "$self->{class_name}::$self->{alias}";
}

sub c_name {
    my $self   = shift;
    my $c_name = "XS_" . $self->perl_name;
    $c_name =~ s/:+/_/g;
    return $c_name;
}

sub c_name_list {
    my $self = shift;
    return $self->{param_list}->name_list;
}

my %params_hash_vals_map = (
    NULL  => 'undef',
    true  => 1,
    false => 0,
);

sub params_hash_def {
    my $self = shift;
    return unless $self->{use_labeled_params};

    my $params_hash_name = $self->perl_name . "_PARAMS";
    my $arg_vars         = $self->{param_list}->get_variables;
    my $vals             = $self->{param_list}->get_initial_values;
    my @pairs;
    for ( my $i = 1; $i < @$arg_vars; $i++ ) {
        my $var = $arg_vars->[$i];
        my $val = $vals->[$i];
        if ( !defined $val ) {
            $val = 'undef';
        }
        elsif ( exists $params_hash_vals_map{$val} ) {
            $val = $params_hash_vals_map{$val};
        }
        push @pairs, $var->micro_sym . " => $val,";
    }

    if (@pairs) {
        my $list = join( "\n    ", @pairs );
        return qq|\%$params_hash_name = (\n    $list\n);\n|;
    }
    else {
        return qq|\%$params_hash_name = ();\n|;
    }
}

my %prim_type_to_allot_macro = (
    double     => 'ALLOT_F64',
    float      => 'ALLOT_F32',
    int        => 'ALLOT_INT',
    short      => 'ALLOT_SHORT',
    long       => 'ALLOT_LONG',
    size_t     => 'ALLOT_SIZE_T',
    uint64_t   => 'ALLOT_U64',
    uint32_t   => 'ALLOT_U32',
    uint16_t   => 'ALLOT_U16',
    uint8_t    => 'ALLOT_U8',
    int64_t    => 'ALLOT_I64',
    int32_t    => 'ALLOT_I32',
    int16_t    => 'ALLOT_I16',
    int8_t     => 'ALLOT_I8',
    chy_bool_t => 'ALLOT_BOOL',
);

sub _allot_params_arg {
    my ( $type, $label, $required ) = @_;
    confess("Not a Clownfish::Type")
        unless blessed($type) && $type->isa('Clownfish::Type');
    my $len = length($label);
    my $req_string = $required ? 'true' : 'false';

    if ( $type->is_object ) {
        my $struct_sym = $type->get_specifier;
        my $vtable     = uc($struct_sym);
        if ( $struct_sym =~ /^[a-z_]*(Obj|CharBuf)$/ ) {
            # Share buffers rather than copy between Perl scalars and
            # Clownfish string types.
            return qq|ALLOT_OBJ(\&$label, "$label", $len, $req_string, |
                . qq|$vtable, alloca(cfish_ZCB_size()))|;
        }
        else {
            return qq|ALLOT_OBJ(\&$label, "$label", $len, $req_string, |
                . qq|$vtable, NULL)|;
        }
    }
    elsif ( $type->is_primitive ) {
        if ( my $allot = $prim_type_to_allot_macro{ $type->to_c } ) {
            return qq|$allot(\&$label, "$label", $len, $req_string)|;
        }
    }

    confess( "Missing typemap for " . $type->to_c );
}

sub build_allot_params {
    my $self         = shift;
    my $param_list   = $self->{param_list};
    my $arg_inits    = $param_list->get_initial_values;
    my $arg_vars     = $param_list->get_variables;
    my $params_hash  = $self->perl_name . "_PARAMS";
    my $allot_params = "";

    # Declare variables and assign default values.
    for ( my $i = 1; $i <= $#$arg_vars; $i++ ) {
        my $arg_var = $arg_vars->[$i];
        my $val     = $arg_inits->[$i];
        if ( !defined($val) ) {
            $val = $arg_var->get_type->is_object ? 'NULL' : '0';
        }
        $allot_params .= $arg_var->local_c . " = $val;\n    ";
    }

    # Iterate over args in param list.
    $allot_params .= qq|chy_bool_t args_ok = XSBind_allot_params(\n|
        . qq|        &(ST(0)), 1, items, "$params_hash",\n|;
    for ( my $i = 1; $i <= $#$arg_vars; $i++ ) {
        my $var      = $arg_vars->[$i];
        my $val      = $arg_inits->[$i];
        my $required = defined $val ? 0 : 1;
        my $name     = $var->micro_sym;
        my $type     = $var->get_type;
        $allot_params .= "        "
            . _allot_params_arg( $type, $name, $required ) . ",\n";
    }
    $allot_params .= qq|        NULL);
    if (!args_ok) {
        CFISH_RETHROW(LUCY_INCREF(cfish_Err_get_error()));
    }|;

    return $allot_params;
}

sub xsub_def { confess "Abstract method" }

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Subroutine - Abstract base binding for a
Clownfish::Function.

=head1 SYNOPSIS

    # Abstract base class.

=head1 DESCRIPTION

This class is used to generate binding code for invoking Clownfish's
functions and methods across the Perl/C barrier.

=head1 METHODS

=head2 new

    my $binding = $subclass->SUPER::new(
        param_list         => $param_list,           # required
        alias              => 'pinch',               # required
        class_name         => 'Crustacean::Claw',    # required
        retval_type        => $type,                 # required
        use_labeled_params => 1,                     # default: false
    );

Abstract constructor.

=over

=item * B<param_list> - A L<Clownfish::ParamList>.

=item * B<alias> - The local, unqualified name for the Perl subroutine that
will be used to invoke the function.

=item * B<class_name> - The name of the Perl class that the subroutine belongs
to.

=item * B<retval_type> - The return value's L<Type|Clownfish::Type>.

=item * B<use_labeled_params> - True if the binding should take hash-style
labeled parameters, false if it should take positional arguments.

=back

=head2 xsub_def

Abstract method which must return C code (not XS code) defining the Perl XSUB.

=head2 get_class_name use_labeled_params

Accessors.

=head2 perl_name

Returns the fully-qualified perl sub name.

=head2 c_name

Returns the fully-qualified name of the C function that implements the XSUB.

=head2 c_name_list

Returns a string containing the names of arguments to feed to bound C
function, joined by commas.

=head2 params_hash_def

Return Perl code initializing a package-global hash where all the keys are the
names of labeled params.  The hash's name consists of the the binding's
perl_name() plus "_PARAMS".

=cut
