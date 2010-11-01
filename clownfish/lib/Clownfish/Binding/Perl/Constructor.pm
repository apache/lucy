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

package Clownfish::Binding::Perl::Constructor;
use base qw( Clownfish::Binding::Perl::Subroutine );
use Carp;
use Clownfish::Binding::Perl::TypeMap qw( from_perl );
use Clownfish::ParamList;

sub new {
    my ( $either, %args ) = @_;
    my $class          = delete $args{class};
    my $alias          = delete $args{alias};
    my $init_func_name = $alias =~ s/^(\w+)\|(\w+)$/$1/ ? $2 : 'init';
    my $class_name     = $class->get_class_name;

    # Find the implementing function.
    my $func;
    for my $function ( $class->functions ) {
        next unless $function->micro_sym eq $init_func_name;
        $func = $function;
        last;
    }
    confess("Missing or invalid init() function for $class_name")
        unless $func;

    my $self = $either->SUPER::new(
        param_list         => $func->get_param_list,
        retval_type        => $func->get_return_type,
        class_name         => $class_name,
        use_labeled_params => 1,
        alias              => $alias,
        %args
    );
    $self->{init_func} = $func;
    return $self;
}

sub xsub_def {
    my $self       = shift;
    my $c_name     = $self->c_name;
    my $param_list = $self->{param_list};
    my $name_list  = $param_list->name_list;
    my $arg_inits  = $param_list->get_initial_values;
    my $num_args   = $param_list->num_vars;
    my $arg_vars   = $param_list->get_variables;
    my $func_sym   = $self->{init_func}->full_func_sym;

    # Create code for allocating labeled parameters.
    my $var_declarations = $self->var_declarations;
    my $params_hash_name = $self->perl_name . "_PARAMS";
    my @var_assignments;
    my @refcount_mods;
    my $allot_params
        = qq|XSBind_allot_params( &(ST(0)), 1, items, "$params_hash_name",\n|;

    # Iterate over args in param list.
    for ( my $i = 1; $i <= $#$arg_vars; $i++ ) {
        my $var     = $arg_vars->[$i];
        my $val     = $arg_inits->[$i];
        my $name    = $var->micro_sym;
        my $sv_name = $name . "_sv";
        my $type    = $var->get_type;
        my $len     = length $name;

        # Create snippet for extracting sv from stack, if supplied.
        $allot_params .= qq|            &$sv_name, "$name", $len,\n|;

        # Create code for determining and validating value.
        my $statement = from_perl( $type, $name, $sv_name );
        if ( defined $val ) {
            my $assignment = qq|if ($sv_name && XSBind_sv_defined($sv_name)) {
            $statement
        }
        else {
            $name = $val;
        }|;
            push @var_assignments, $assignment;
        }
        else {
            my $assignment
                = qq#if ( !$sv_name || !XSBind_sv_defined($sv_name) ) {
           CFISH_THROW(KINO_ERR, "Missing required param '$name'");
        }
        $statement#;
            push @var_assignments, $assignment;
        }

        # Compensate for the fact that the method will swallow a refcount.
        if ( $type->is_object and $type->decremented ) {
            push @refcount_mods, "if ($name) { LUCY_INCREF($name); }";
        }
    }
    $allot_params .= "            NULL);\n";

    # Last, so that earlier exceptions while fetching params don't trigger bad
    # DESTROY.
    my $self_var  = $arg_vars->[0];
    my $self_type = $self_var->get_type->to_c;
    push @var_assignments,
        qq|self = ($self_type)XSBind_new_blank_obj( ST(0) );|;

    # Bundle up variable assignment statments.
    my $var_assignments
        = join( "\n        ", $allot_params, @var_assignments );
    my $refcount_mods = join( "\n        ", @refcount_mods );

    return <<END_STUFF;
XS($c_name);
XS($c_name)
{
    dXSARGS;
    CHY_UNUSED_VAR(cv);
    CHY_UNUSED_VAR(ax);
    if (items < 1) { CFISH_THROW(KINO_ERR, "Usage: %s(class_name, ...)",  GvNAME(CvGV(cv))); }
    SP -= items;
    {
        $var_declarations
        $var_assignments
        $refcount_mods
        retval = $func_sym($name_list);
        if (retval) {
            ST(0) = (SV*)Kino_Obj_To_Host((kino_Obj*)retval);
            Kino_Obj_Dec_RefCount((kino_Obj*)retval);
        }
        else {
            ST(0) = newSV(0);
        }
        sv_2mortal( ST(0) );
        XSRETURN(1);
    }
    PUTBACK;
}

END_STUFF
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Constructor - Binding for an object method.

=head1 DESCRIPTION

This class isa Clownfish::Binding::Perl::Subroutine -- see its
documentation for various code-generating routines.

Constructors are always bound to accept labeled params, even if there is only
a single argument.

=head1 METHODS

=head2 new

    my $constructor_binding = Clownfish::Binding::Perl::Constructor->new(
        class => $class,
        alias => "_new|init2",
    );

=over

=item * B<class> - A L<Clownfish::Class>.

=item * B<alias> - A specifier for the name of the constructor, and
optionally, a specifier for the implementing function.  If C<alias> has a pipe
character in it, the text to the left of the pipe will be used as the Perl
alias, and the text to the right will be used to determine which C function
should be bound.  The default function is "init".

=back

=head2 xsub_def

Generate the XSUB code.

=cut
