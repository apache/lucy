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

package Clownfish::Binding::Core::Class;
use Clownfish::Util qw( a_isa_b verify_args );
use Clownfish::Binding::Core::Method;
use Clownfish::Binding::Core::Function;
use File::Spec::Functions qw( catfile );

our %new_PARAMS = ( client => undef, );

sub new {
    my ( $either, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    return _new( $args{client} );
}

# C code defining the ZombieCharBuf which contains the class name for this
# class.
sub _name_var_definition {
    my $self           = shift;
    my $full_var_name  = _full_name_var($self);
    my $class_name     = $self->_get_client->get_class_name;
    my $class_name_len = length($class_name);
    return <<END_STUFF;
cfish_ZombieCharBuf $full_var_name = {
    CFISH_ZOMBIECHARBUF,
    {1}, /* ref.count */
    "$class_name",
    $class_name_len,
    0
};

END_STUFF
}

# Return C code defining the class's VTable.
sub _vtable_definition {
    my $self       = shift;
    my $client     = $self->_get_client;
    my $parent     = $client->get_parent;
    my $methods    = $client->methods;
    my $vt_type    = $client->full_vtable_type;
    my $cnick      = $client->get_cnick;
    my $vtable_var = $client->full_vtable_var;
    my $struct_sym = $client->full_struct_sym;
    my $vt         = $vtable_var . "_vt";
    my $name_var   = _full_name_var($self);
    my $cb_var     = _full_callbacks_var($self);

    # Create a pointer to the parent class's vtable.
    my $parent_ref
        = defined $parent
        ? $parent->full_vtable_var
        : "NULL";    # No parent, e.g. Obj or inert classes.

    # Spec functions which implement the methods, casting to quiet compiler.
    my @implementing_funcs
        = map { "(cfish_method_t)" . $_->full_func_sym } @$methods;
    my $method_string = join( ",\n        ", @implementing_funcs );
    my $num_methods = scalar @implementing_funcs;

    return <<END_VTABLE

$vt_type $vt = {
    CFISH_VTABLE, /* vtable vtable */
    {1}, /* ref.count */
    $parent_ref, /* parent */
    (cfish_CharBuf*)&$name_var,
    0, /* flags */
    NULL, /* "void *x" member reserved for future use */
    sizeof($struct_sym), /* obj_alloc_size */
    offsetof(cfish_VTable, methods)
        + $num_methods * sizeof(cfish_method_t), /* vt_alloc_size */
    &$cb_var,  /* callbacks */
    {
        $method_string
    }
};

END_VTABLE
}

# Create the definition for the instantiable object struct.
sub _struct_definition {
    my $self                = shift;
    my $struct_sym          = $self->_get_client->full_struct_sym;
    my $member_declarations = join( "\n    ",
        map { $_->local_declaration } @{ $self->_get_client->member_vars } );
    return <<END_STRUCT
struct $struct_sym {
    $member_declarations
};
END_STRUCT
}

sub to_c_header {
    my $self          = shift;
    my $client        = $self->_get_client;
    my $cnick         = $client->get_cnick;
    my $functions     = $client->functions;
    my $methods       = $client->methods;
    my $novel_methods = $client->novel_methods;
    my $inert_vars    = $client->inert_vars;
    my $vtable_var    = $client->full_vtable_var;
    my $short_vt_var  = $client->short_vtable_var;
    my $short_struct  = $client->get_struct_sym;
    my $full_struct   = $client->full_struct_sym;
    my $c_file_sym    = "C_" . uc($full_struct);
    my $struct_def    = _struct_definition($self);

    # If class inherits from something, include the parent class's header.
    my $parent_include = "";
    if ( my $parent = $client->get_parent ) {
        $parent_include = $parent->include_h;
        $parent_include = qq|#include "$parent_include"|;
    }

    # Add a C function definition for each method and each function.
    my $sub_declarations = "";
    for my $sub ( @$functions, @$novel_methods ) {
        $sub_declarations
            .= Clownfish::Binding::Core::Function->func_declaration($sub)
            . "\n\n";
    }

    # Declare class (a.k.a. "inert") variables.
    my $inert_var_defs = "";
    for my $inert_var (@$inert_vars) {
        $inert_var_defs .= "extern " . $inert_var->global_c . ";\n";
    }

    # Declare typedefs for novel methods, to ease casting.
    my $method_typedefs = '';
    for my $method (@$novel_methods) {
        $method_typedefs
            .= Clownfish::Binding::Core::Method->typedef_dec($method) . "\n";
    }

    # Define method invocation syntax.
    my $method_defs = '';
    for my $method (@$methods) {
        $method_defs .= Clownfish::Binding::Core::Method->method_def(
            method => $method,
            class  => $self->_get_client,
        ) . "\n";
    }

    # Declare the virtual table singleton object.
    my $vt_type = $self->_get_client->full_vtable_type;
    my $vt      = "extern struct $vt_type ${vtable_var}_vt;";
    my $vtable_object
        = "#define $vtable_var ((cfish_VTable*)&${vtable_var}_vt)";
    my $num_methods = scalar @$methods;

    # Declare cfish_Callback objects.
    my $callback_declarations = "";
    for my $method (@$novel_methods) {
        next unless $method->public || $method->abstract;
        $callback_declarations
            .= Clownfish::Binding::Core::Method->callback_dec($method);
    }

    # Define short names.
    my $short_names       = '';
    my $short_names_macro = _short_names_macro($self);
    for my $function (@$functions) {
        my $short_func_sym = $function->short_sym;
        my $full_func_sym  = $function->full_sym;
        $short_names .= "  #define $short_func_sym $full_func_sym\n";
    }
    for my $inert_var (@$inert_vars) {
        my $short_sym = $inert_var->short_sym;
        my $full_sym  = $inert_var->full_sym;
        $short_names .= "  #define $short_sym $full_sym\n";
    }
    if ( !$client->inert ) {
        for my $method (@$novel_methods) {
            if ( !$method->isa("Clownfish::Method::Overridden") ) {
                my $short_typedef = $method->short_typedef;
                my $full_typedef  = $method->full_typedef;
                $short_names .= "  #define $short_typedef $full_typedef\n";
            }
            my $short_func_sym = $method->short_func_sym;
            my $full_func_sym  = $method->full_func_sym;
            $short_names .= "  #define $short_func_sym $full_func_sym\n";
        }
        for my $method (@$methods) {
            my $short_method_sym = $method->short_method_sym($cnick);
            my $full_method_sym  = $method->full_method_sym($cnick);
            $short_names .= "  #define $short_method_sym $full_method_sym\n";
        }
    }

    # Make the spacing in the file a little more elegant.
    s/\s+$// for ( $method_typedefs, $method_defs, $short_names );

    # Inert classes only output inert functions and member vars.
    if ( $client->inert ) {
        return <<END_INERT
#include "charmony.h"
#include "boil.h"
$parent_include

$inert_var_defs

$sub_declarations

#ifdef $short_names_macro
$short_names
#endif /* $short_names_macro */

END_INERT
    }

    # Instantiable classes get everything.
    return <<END_STUFF;

#include "charmony.h"
#include "boil.h"
$parent_include

#ifdef $c_file_sym
$struct_def
#endif /* $c_file_sym */

$inert_var_defs

$sub_declarations
$callback_declarations

$method_typedefs

$method_defs

typedef struct $vt_type {
    cfish_VTable *vtable;
    cfish_ref_t ref;
    cfish_VTable *parent;
    cfish_CharBuf *name;
    uint32_t flags;
    void *x;
    size_t obj_alloc_size;
    size_t vt_alloc_size;
    void *callbacks;
    cfish_method_t methods[$num_methods];
} $vt_type;
$vt
$vtable_object

#ifdef $short_names_macro
  #define $short_struct $full_struct
  #define $short_vt_var $vtable_var
$short_names
#endif /* $short_names_macro */

END_STUFF
}

sub to_c {
    my $self   = shift;
    my $client = $self->_get_client;

    return $client->get_autocode if $client->inert;

    my $include_h      = $client->include_h;
    my $autocode       = $client->get_autocode;
    my $offsets        = '';
    my $abstract_funcs = '';
    my $callback_funcs = '';
    my $callbacks      = '';
    my $vt_type        = $client->full_vtable_type;
    my $meth_num       = 0;
    my $cnick          = $client->get_cnick;
    my $class_name_def = _name_var_definition($self);
    my $vtable_def     = _vtable_definition($self);
    my @class_callbacks;

    # Prepare to identify novel methods.
    my %novel = map { ( $_->micro_sym => $_ ) } @{ $client->novel_methods };

    for my $method ( @{ $client->methods } ) {
        my $var_name = $method->full_offset_sym($cnick);

        # Create offset in bytes for the method from the top of the VTable
        # object.
        my $offset = "(offsetof($vt_type, methods)"
            . " + $meth_num * sizeof(cfish_method_t))";
        $offsets .= "size_t $var_name = $offset;\n";

        # Create a default implementation for abstract methods.
        if ( $method->abstract ) {
            if ( $novel{ $method->micro_sym } ) {
                $callback_funcs
                    .= Clownfish::Binding::Core::Method->abstract_method_def(
                    $method)
                    . "\n";
            }
        }

        # Define callbacks for methods that can be overridden via the
        # host.
        if ( $method->public or $method->abstract ) {
            my $callback_sym = $method->full_callback_sym;
            if ( $novel{ $method->micro_sym } ) {
                $callback_funcs
                    .= Clownfish::Binding::Core::Method->callback_def($method)
                    . "\n";
                $callbacks
                    .= Clownfish::Binding::Core::Method->callback_obj_def(
                    method => $method,
                    offset => $offset,
                    );
            }
            push @class_callbacks, "&$callback_sym";
        }
        $meth_num++;
    }

    # Create a NULL-terminated array of cfish_Callback vars.  Since C89
    # doesn't allow us to initialize a pointer to an anonymous array inside a
    # global struct, we have to give it a real symbol and then store a pointer
    # to that symbol inside the VTable struct.
    my $callbacks_var = _full_callbacks_var($self);
    $callbacks .= "cfish_Callback *$callbacks_var" . "[] = {\n    ";
    $callbacks .= join( ",\n    ", @class_callbacks, "NULL" );
    $callbacks .= "\n};\n";

    return <<END_STUFF;
#include "$include_h"

$offsets
$callback_funcs
$callbacks
$class_name_def
$vtable_def
$autocode

END_STUFF
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Core::Class - Generate core C code for a class.

=head1 DESCRIPTION

Clownfish::Class is an abstract specification for a class.  This module
autogenerates the C code with implements that specification.

=head1 METHODS

=head2 new

    my $class_binding = Clownfish::Binding::Core::Class->new(
        client => $class,
    );

=over

=item * B<client> - A L<Clownfish::Class>.

=back

=head2 to_c_header

Return the .h file which contains autogenerated C code defining the class's
interface:  all method invocation functions, etc...

=head2 to_c

Return the .c file which contains autogenerated C code necessary for the class
to function properly.

=cut
