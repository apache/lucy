use strict;
use warnings;

package Clownfish::Binding::Perl::Method;
use base qw( Clownfish::Binding::Perl::Subroutine );
use Clownfish::Util qw( verify_args );
use Clownfish::Binding::Perl::TypeMap qw( from_perl to_perl );
use Carp;

our %new_PARAMS = ( 
    method => undef,
    alias  => undef,
);

sub new {
    my ( $either, %args ) = @_;
    confess $@ unless verify_args( \%new_PARAMS, %args );

    # Derive arguments to SUPER constructor from supplied Method.
    my $method = delete $args{method};
    $args{retval_type} ||= $method->get_return_type;
    $args{param_list}  ||= $method->get_param_list;
    $args{alias}       ||= $method->micro_sym;
    $args{class_name}  ||= $method->get_class_name;
    if ( !defined $args{use_labeled_params} ) {
        $args{use_labeled_params}
            = $method->get_param_list->num_vars > 2
            ? 1
            : 0;
    }
    my $self = $either->SUPER::new(%args);
    $self->{method} = $method;

    return $self;
}

sub xsub_def {
    my $self = shift;
    if ( $self->{use_labeled_params} ) {
        return $self->_xsub_def_labeled_params;
    }
    else {
        return $self->_xsub_def_positional_args;
    }
}

# Build XSUB function body.
sub _xsub_body {
    my $self           = shift;
    my $method         = $self->{method};
    my $full_func_sym  = $method->full_func_sym;
    my $param_list     = $method->get_param_list;
    my $arg_vars       = $param_list->get_variables;
    my $name_list      = $param_list->name_list;
    my $body           = "";

    # Compensate for functions which eat refcounts.
    for my $arg_var (@$arg_vars) {
        my $arg_type = $arg_var->get_type;
        next unless $arg_type->is_object;
        next unless $arg_type->decremented;
        my $var_name = $arg_var->micro_sym;
        $body .= "if ($var_name) (void)LUCY_INCREF($var_name);\n        ";
    }

    if ( $method->void ) {
        # Invoke method in void context.
        $body .= qq|$full_func_sym($name_list);|;
    }
    else {
        # Return a value for method invoked in a scalar context.
        my $return_type       = $method->get_return_type;
        my $retval_assignment = to_perl( $return_type, 'ST(0)', 'retval' );
        my $decrement         = "";
        if ( $return_type->is_object and $return_type->incremented ) {
            $decrement = "LUCY_DECREF(retval);\n";
        }
        $body .= qq|retval = $full_func_sym($name_list);
        $retval_assignment$decrement
        sv_2mortal( ST(0) );
        XSRETURN(1);|
    }

    return $body;
}

sub _xsub_def_positional_args {
    my $self       = shift;
    my $method     = $self->{method};
    my $param_list = $method->get_param_list;
    my $arg_vars   = $param_list->get_variables;
    my $arg_inits  = $param_list->get_initial_values;
    my $num_args   = $param_list->num_vars;
    my $c_name     = $self->c_name;
    my $body       = $self->_xsub_body;

    # Determine how many args are truly required and build an error check.
    my $min_required = $num_args;
    while ( defined $arg_inits->[ $min_required - 1 ] ) {
        $min_required--;
    }
    my @xs_arg_names;
    for ( my $i = 0; $i < $min_required; $i++ ) {
        push @xs_arg_names, $arg_vars->[$i]->micro_sym;
    }
    my $xs_name_list = join( ", ", @xs_arg_names );
    my $num_args_check;
    if ( $min_required < $num_args ) {
        $num_args_check
            = qq|if (items < $min_required) { |
            . qq|THROW(LUCY_ERR, "Usage: %s(%s)",  GvNAME(CvGV(cv)),|
            . qq| "$xs_name_list"); }|;
    }
    else {
        $num_args_check
            = qq|if (items != $num_args) { |
            . qq| THROW(LUCY_ERR, "Usage: %s(%s)",  GvNAME(CvGV(cv)), |
            . qq|"$xs_name_list"); }|;
    }

    # Var assignments.
    my $var_declarations = $self->var_declarations;
    my @var_assignments;
    for ( my $i = 0; $i < @$arg_vars; $i++ ) {
        my $var        = $arg_vars->[$i];
        my $val        = $arg_inits->[$i];
        my $var_name   = $var->micro_sym;
        my $stack_name = $var_name . '_zcb';
        my $var_type   = $var->get_type;
        my $statement;
        if ( $i == 0 ) {    # $self
            $statement
                = _self_assign_statement( $var_type, $method->micro_sym );
        }
        else {
            $statement
                = from_perl( $var_type, $var_name, "ST($i)", $stack_name );
        }
        if ( defined $val ) {
            $statement
                = qq|    if ( items >= $i && XSBind_sv_defined(ST($i)) ) {
            $statement
        }
        else { 
            $var_name = $val;
        }|;
        }
        push @var_assignments, $statement;
    }
    my $var_assignments = join "\n    ", @var_assignments;

    return <<END_STUFF;
XS($c_name); /* -Wmissing-prototypes */
XS($c_name)
{
    dXSARGS;
    CHY_UNUSED_VAR(cv);
    CHY_UNUSED_VAR(ax);
    SP -= items;
    $num_args_check;

    {
        /* Extract vars from Perl stack. */
        $var_declarations
        $var_assignments

        /* Execute */
        $body
    }

    PUTBACK;
}
END_STUFF
}

sub _xsub_def_labeled_params {
    my $self       = shift;
    my $c_name     = $self->c_name;
    my $param_list = $self->{param_list};
    my $arg_inits  = $param_list->get_initial_values;
    my $num_args   = $param_list->num_vars;
    my $arg_vars   = $param_list->get_variables;
    my $body       = $self->_xsub_body;

    # Prepare error message for incorrect args.
    my $name_list = $arg_vars->[0]->micro_sym . ", ...";
    my $num_args_check
        = qq|if (items < 1) { |
        . qq|THROW(LUCY_ERR, "Usage: %s(%s)",  GvNAME(CvGV(cv)), |
        . qq|"$name_list"); }|;

    # Create code for allocating labeled parameters.
    my $var_declarations = $self->var_declarations;
    my $self_var         = $arg_vars->[0];
    my $self_type        = $self_var->get_type;
    my $params_hash_name = $self->perl_name . "_PARAMS";
    my $self_assignment
        = _self_assign_statement( $self_type, $self->{method}->micro_sym );
    my @var_assignments;
    my $allot_params
        = qq|XSBind_allot_params( &(ST(0)), 1, items, "$params_hash_name", |;

    # Iterate over args in param list.
    for ( my $i = 1; $i <= $#$arg_vars; $i++ ) {
        my $var        = $arg_vars->[$i];
        my $val        = $arg_inits->[$i];
        my $name       = $var->micro_sym;
        my $sv_name    = $name . "_sv";
        my $stack_name = $name . "_zcb";
        my $type       = $var->get_type;
        my $len        = length $name;

        # Code for extracting sv from stack, if supplied.
        $allot_params .= qq|            &$sv_name, "$name", $len,\n|;

        # Code for determining and validating value.
        my $statement = from_perl( $type, $name, $sv_name, $stack_name );
        if ( defined $val ) {
            my $assignment
                = qq|if ( $sv_name && XSBind_sv_defined($sv_name) ) {
            $statement;
        }
        else {
            $name = $val;
        }|;
            push @var_assignments, $assignment;
        }
        else {
            my $assignment
                = qq#if ( !$sv_name || !XSBind_sv_defined($sv_name) ) { #
                . qq#THROW(LUCY_ERR, "Missing required param '$name'"); }\n#
                . qq#         $statement;#;
            push @var_assignments, $assignment;
        }
    }
    $allot_params .= " NULL);\n";
    my $var_assignments = join( "\n        ",
        $self_assignment, $allot_params, @var_assignments, );

    return <<END_STUFF;
XS($c_name); /* -Wmissing-prototypes */
XS($c_name)
{
    dXSARGS;
    CHY_UNUSED_VAR(cv);
    CHY_UNUSED_VAR(ax);
    $num_args_check;
    SP -= items;

    {
        /* Extract vars from Perl stack. */
        $var_declarations
        $var_assignments

        /* Execute */
        $body
    }

    PUTBACK;
}
END_STUFF
}

# Create an assignment statement for extracting $self from the Perl stack.
sub _self_assign_statement {
    my ( $type, $method_name ) = @_;
    my $type_c = $type->to_c;
    $type_c =~ /(\w+)\*$/ or die "Not an object type: $type_c";
    my $vtable = uc($1);

    # Make an exception for deserialize -- allow self to be NULL if called as
    # a class method.
    my $binding_func
        = $method_name eq 'deserialize'
        ? 'XSBind_maybe_sv_to_lucy_obj'
        : 'XSBind_sv_to_lucy_obj';
    return "self = ($type_c)$binding_func(ST(0), $vtable, NULL);";
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Method - Binding for an object method.

=head1 DESCRIPTION

This class isa Clownfish::Binding::Perl::Subroutine -- see its
documentation for various code-generating routines.

Method bindings use labeled parameters if the C function takes more than one
argument (other than C<self>).  If there is only one argument, the binding
will be set up to accept a single positional argument.

=head1 METHODS

=head2 new

    my $binding = Clownfish::Binding::Perl::Method->new(
        method => $method,    # required
    );

=over

=item * B<method> - A L<Clownfish::Method>.

=back

=head2 xsub_def

Generate the XSUB code.

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

