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
