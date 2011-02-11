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

package Clownfish::Binding::Core::Method;
use Clownfish::Util qw( a_isa_b );
use Carp;

sub method_def {
    my ( undef,   %args )  = @_;
    my ( $method, $class ) = @args{qw( method class )};
    confess("Not a Method")
        unless a_isa_b( $method, "Clownfish::Method" );
    confess("Not a Class")
        unless a_isa_b( $class, "Clownfish::Class" );
    if ( $method->final ) {
        return _final_method_def( $method, $class );
    }
    else {
        return _virtual_method_def( $method, $class );
    }
}

sub _virtual_method_def {
    my ( $method, $class ) = @_;
    my $cnick           = $class->get_cnick;
    my $param_list      = $method->get_param_list;
    my $invoker_struct  = $class->full_struct_sym;
    my $common_struct   = $method->self_type->get_specifier;
    my $full_method_sym = $method->full_method_sym($cnick);
    my $full_offset_sym = $method->full_offset_sym($cnick);
    my $typedef         = $method->full_typedef;
    my $arg_names       = $param_list->name_list;
    $arg_names =~ s/\s*\w+/self/;

    # Prepare the parameter list for the inline function.
    my $params = $param_list->to_c;
    $params =~ s/^.*?\*\s*\w+/const $invoker_struct *self/
        or confess("no match: $params");

    # Prepare a return statement... or not.
    my $return_type = $method->get_return_type->to_c;
    my $maybe_return = $method->get_return_type->is_void ? '' : 'return ';

    return <<END_STUFF;
extern size_t $full_offset_sym;
static CHY_INLINE $return_type
$full_method_sym($params)
{
    char *const method_address = *(char**)self + $full_offset_sym;
    const $typedef method = *(($typedef*)method_address);
    ${maybe_return}method(($common_struct*)$arg_names);
}
END_STUFF
}

# Create a macro definition that aliases to a function name directly, since
# this method may not be overridden.
sub _final_method_def {
    my ( $method, $class ) = @_;
    my $cnick           = $class->get_cnick;
    my $macro_sym       = $method->get_macro_sym;
    my $self_type       = $method->self_type->to_c;
    my $full_method_sym = $method->full_method_sym($cnick);
    my $full_func_sym   = $method->full_func_sym;
    my $arg_names       = $method->get_param_list->name_list;

    return <<END_STUFF;
#define $full_method_sym($arg_names) \\
    $full_func_sym(($self_type)$arg_names)
END_STUFF
}

sub typedef_dec {
    my ( undef, $method ) = @_;
    my $params      = $method->get_param_list->to_c;
    my $return_type = $method->get_return_type->to_c;
    my $typedef     = $method->full_typedef;
    return <<END_STUFF;
typedef $return_type
(*$typedef)($params);
END_STUFF
}

sub callback_dec {
    my ( undef, $method ) = @_;
    my $callback_sym = $method->full_callback_sym;
    return qq|extern cfish_Callback $callback_sym;\n|;
}

sub callback_obj_def {
    my ( undef, %args ) = @_;
    my $method       = $args{method};
    my $offset       = $args{offset};
    my $macro_sym    = $method->get_macro_sym;
    my $len          = length($macro_sym);
    my $func_sym     = $method->full_override_sym;
    my $callback_sym = $method->full_callback_sym;
    return qq|cfish_Callback $callback_sym = |
        . qq|{"$macro_sym", $len, (cfish_method_t)$func_sym, $offset};\n|;
}

sub callback_def {
    my ( undef, $method ) = @_;
    my $return_type = $method->get_return_type;
    my $params      = _callback_params($method);
    if ( !$params ) {
        # Can't map vars, because there's at least one type in the argument
        # list we don't yet support.  Return a callback wrapper that throws an
        # error error.
        return _invalid_callback_def( $method, $params );
    }
    elsif ( $return_type->is_void ) {
        return _void_callback_def( $method, $params );
    }
    elsif ( $return_type->is_object ) {
        return _obj_callback_def( $method, $params );
    }
    else {
        return _primitive_callback_def( $method, $params );
    }
}

# Return a string which maps arguments to various arg wrappers conforming
# to Host's callback interface.  For instance, (int32_t foo, Obj *bar)
# produces the following:
#
#   CFISH_ARG_I32("foo", foo),
#   CFISH_ARG_OBJ("bar", bar)
#
sub _callback_params {
    my $method     = shift;
    my $micro_sym  = $method->micro_sym;
    my $param_list = $method->get_param_list;
    my $num_params = $param_list->num_vars - 1;
    my $arg_vars   = $param_list->get_variables;
    my @params;

    # Iterate over arguments, mapping them to various arg wrappers which
    # conform to Host's callback interface.
    for my $var ( @$arg_vars[ 1 .. $#$arg_vars ] ) {
        my $name = $var->micro_sym;
        my $type = $var->get_type;
        my $param;
        if ( $type->is_string_type ) {
            $param = qq|CFISH_ARG_STR("$name", $name)|;
        }
        elsif ( $type->is_object ) {
            $param = qq|CFISH_ARG_OBJ("$name", $name)|;
        }
        elsif ( $type->is_integer ) {
            my $width = $type->get_width;
            if ($width) {
                if ($width <= 4) {
                    $param = qq|CFISH_ARG_I32("$name", $name)|;
                }
                else {
                    $param = qq|CFISH_ARG_I64("$name", $name)|;
                }
            }
            else {
                my $c_type = $type->to_c;
                $param = qq|CFISH_ARG_I($c_type, "$name", $name)|;
            }
        }
        elsif ( $type->is_floating ) {
            $param = qq|CFISH_ARG_F64("$name", $name)|;
        }
        else {
            # Can't map variable type.  Signal to caller.
            return undef;
        }
        push @params, $param;
    }
    return join( ', ', 'self', qq|"$micro_sym"|, $num_params, @params );
}

# Return a function which throws a runtime error indicating which variable
# couldn't be mapped.  TODO: it would be better to resolve all these cases at
# compile-time.
sub _invalid_callback_def {
    my ( $method, $callback_params ) = @_;
    my $full_method_sym
        = $method->full_method_sym( $method->get_class_cnick );
    my $override_sym = $method->full_override_sym;
    my $params       = $method->get_param_list->to_c;
    my $unused       = '';
    for my $var ( @{ $method->get_param_list->get_variables } ) {
        my $var_name = $var->micro_sym;
        $unused .= "CHY_UNUSED_VAR($var_name); ";
    }
    return <<END_CALLBACK_DEF;
void
$override_sym($params)
{
    $unused;
    CFISH_THROW(CFISH_ERR, "Can't override $full_method_sym via binding");
}
END_CALLBACK_DEF
}

# Create a callback for a method which operates in a void context.
sub _void_callback_def {
    my ( $method, $callback_params ) = @_;
    my $override_sym = $method->full_override_sym;
    my $params       = $method->get_param_list->to_c;
    return <<END_CALLBACK_DEF;
void
$override_sym($params)
{
    cfish_Host_callback($callback_params);
}
END_CALLBACK_DEF
}

# Create a callback which returns a primitive type.
sub _primitive_callback_def {
    my ( $method, $callback_params ) = @_;
    my $override_sym    = $method->full_override_sym;
    my $params          = $method->get_param_list->to_c;
    my $return_type     = $method->get_return_type;
    my $return_type_str = $return_type->to_c;
    my $nat_func
        = $return_type->is_floating ? "cfish_Host_callback_f64"
        : $return_type->is_integer  ? "cfish_Host_callback_i64"
        : $return_type_str eq 'void*' ? "cfish_Host_callback_host"
        :   confess("unrecognized type: $return_type_str");
    return <<END_CALLBACK_DEF;
$return_type_str
$override_sym($params)
{
    return ($return_type_str)$nat_func($callback_params);
}
END_CALLBACK_DEF
}

# Create a callback which returns an object type -- either a generic object or
# a string.
sub _obj_callback_def {
    my ( $method, $callback_params ) = @_;
    my $override_sym    = $method->full_override_sym;
    my $params          = $method->get_param_list->to_c;
    my $return_type     = $method->get_return_type;
    my $return_type_str = $return_type->to_c;
    my $cb_func_name
        = $return_type->is_string_type
        ? "cfish_Host_callback_str"
        : "cfish_Host_callback_obj";

    my $nullable_check = "";
    if ( !$return_type->nullable ) {
        my $macro_sym = $method->get_macro_sym;
        $nullable_check
            = qq|if (!retval) { CFISH_THROW(CFISH_ERR, |
            . qq|"$macro_sym() for class '%o' cannot return NULL", |
            . qq|Cfish_Obj_Get_Class_Name((cfish_Obj*)self)); }\n    |;
    }

    my $decrement = "";
    if ( !$return_type->incremented ) {
        $decrement = "LUCY_DECREF(retval);\n    ";
    }

    return <<END_CALLBACK_DEF;
$return_type_str
$override_sym($params)
{
    $return_type_str retval = ($return_type_str)$cb_func_name($callback_params);
    ${nullable_check}${decrement}return retval;
}
END_CALLBACK_DEF
}

# Create a function which throws a runtime error indicating that a method is
# abstract.  This serves as the implementation for methods which are
# declared as "abstract" in a Clownfish header file.
sub abstract_method_def {
    my ( undef, $method ) = @_;
    my $params          = $method->get_param_list->to_c;
    my $full_func_sym   = $method->full_func_sym;
    my $vtable          = uc( $method->self_type->get_specifier );
    my $return_type     = $method->get_return_type;
    my $return_type_str = $return_type->to_c;
    my $macro_sym       = $method->get_macro_sym;

    # Build list of unused params and create an unreachable return statement
    # if necessary, in order to thwart compiler warnings.
    my $param_vars = $method->get_param_list->get_variables;
    my $unused     = "";
    for ( my $i = 1; $i < @$param_vars; $i++ ) {
        my $var_name = $param_vars->[$i]->micro_sym;
        $unused .= "\n    CHY_UNUSED_VAR($var_name);";
    }
    my $ret_statement = '';
    if ( !$return_type->is_void ) {
        $ret_statement = "\n    CHY_UNREACHABLE_RETURN($return_type_str);";
    }

    return <<END_ABSTRACT_DEF;
$return_type_str
$full_func_sym($params)
{
    cfish_CharBuf *klass = self ? Cfish_Obj_Get_Class_Name((cfish_Obj*)self) : $vtable->name;$unused
    CFISH_THROW(CFISH_ERR, "Abstract method '$macro_sym' not defined by %o", klass);$ret_statement
}
END_ABSTRACT_DEF
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Core::Method - Generate core C code for a method.

=head1 DESCRIPTION

Clownfish::Method is an abstract specification; this class generates C code
which implements the specification.

=head1 METHODS

=head2 method_def

    my $c_code = Clownfish::Binding::Core::Method->method_def(
        method => $method,
        $class => $class,
    );

Return C code for the static inline vtable method invocation function.  

=over

=item * B<method> - A L<Clownfish::Method>.

=item * B<class> - The L<Clownfish::Class> which will be invoking the method -
LobsterClaw needs its own method invocation function even if the method was
defined in Claw.

=back

=head2 typedef_dec

    my $c_code = Clownfish::Binding::Core::Method->typedef_dec($method);

Return C code expressing a typedef declaration for the method.

=head2 callback_dec

    my $c_code = Clownfish::Binding::Core::Method->callback_dec($method);

Return C code declaring the Callback object for this method.

=head2 callback_obj_def

    my $c_code 
        = Clownfish::Binding::Core::Method->callback_obj_def($method);

Return C code defining the Callback object for this method, which stores
introspection data and a pointer to the callback function.

=head2 callback_def

    my $c_code = Clownfish::Binding::Core::Method->callback_def($method);

Return C code implementing a callback to the Host for this method.  This code
is used when a Host method has overridden a method in a Clownfish class.

=head2 abstract_method_def

    my $c_code 
        = Clownfish::Binding::Core::Method->abstract_method_def($method);

Return C code implementing a version of the method which throws an "abstract
method" error at runtime.

=cut
