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
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_, }, ref($either) || $either;

    # Validate.
    my $client = $self->{client};
    confess("Not a Clownfish::Class")
        unless a_isa_b( $client, "Clownfish::Class" );

    # Cache some vars.
    $self->{class_name}      = $client->get_class_name;
    $self->{full_struct_sym} = $client->full_struct_sym;
    $self->{struct_sym}      = $client->get_struct_sym;
    $self->{cnick}           = $client->get_cnick;
    $self->{source_class}    = $client->get_source_class;

    return $self;
}

sub get_prefix { shift->{client}->get_prefix }
sub get_Prefix { shift->{client}->get_Prefix }
sub get_PREFIX { shift->{client}->get_PREFIX }

sub file_path {
    my ( $self, $base_dir, $ext ) = @_;
    my @components = split( '::', $self->{source_class} );
    unshift @components, $base_dir
        if defined $base_dir;
    $components[-1] .= $ext;
    return catfile(@components);
}

sub include_h {
    my $self = shift;
    my @components = split( '::', $self->{source_class} );
    $components[-1] .= '.h';
    return join( '/', @components );
}

sub vtable_var    { uc( shift->{struct_sym} ) }
sub callbacks_var { shift->vtable_var . '_CALLBACKS' }
sub name_var      { shift->vtable_var . '_CLASS_NAME' }

sub name_var_definition {
    my $self           = shift;
    my $prefix         = $self->get_prefix;
    my $PREFIX         = $self->get_PREFIX;
    my $full_var_name  = $PREFIX . $self->name_var;
    my $class_name_len = length( $self->{class_name} );
    return <<END_STUFF;
${prefix}ZombieCharBuf $full_var_name = {
    ${PREFIX}ZOMBIECHARBUF,
    {1}, /* ref.count */
    "$self->{class_name}",
    $class_name_len,
    0
};

END_STUFF
}

sub vtable_type { shift->vtable_var . '_VT' }

sub vtable_definition {
    my $self       = shift;
    my $client     = $self->{client};
    my $parent     = $client->get_parent;
    my @methods    = $client->methods;
    my $name_var   = $self->name_var;
    my $vtable_var = $self->vtable_var;
    my $vt         = $vtable_var . "_vt";
    my $vt_type    = $self->vtable_type;
    my $cnick      = $self->{cnick};
    my $prefix     = $self->get_prefix;
    my $PREFIX     = $self->get_PREFIX;

    # Create a pointer to the parent class's vtable.
    my $parent_ref
        = defined $parent
        ? "$PREFIX" . $parent->vtable_var
        : "NULL";    # No parent, e.g. Obj or inert classes.

    # Spec functions which implement the methods, casting to quiet compiler.
    my @implementing_funcs
        = map { "(kino_method_t)" . $_->full_func_sym } @methods;
    my $method_string = join( ",\n        ", @implementing_funcs );
    my $num_methods = scalar @implementing_funcs;

    return <<END_VTABLE

$PREFIX$vt_type $PREFIX$vt = {
    ${PREFIX}VTABLE, /* vtable vtable */
    {1}, /* ref.count */
    $parent_ref, /* parent */
    (${prefix}CharBuf*)&${PREFIX}$name_var,
    0, /* flags */
    NULL, /* "void *x" member reserved for future use */
    sizeof($self->{full_struct_sym}), /* obj_alloc_size */
    offsetof(${prefix}VTable, methods) 
        + $num_methods * sizeof(kino_method_t), /* vt_alloc_size */
    (${prefix}Callback**)&${PREFIX}${vtable_var}_CALLBACKS,  /* callbacks */
    {
        $method_string
    }
};

END_VTABLE
}

sub struct_definition {
    my $self   = shift;
    my $prefix = $self->get_prefix;

    # Add a line for each member var.
    my $member_declarations = join( "\n    ",
        map { $_->local_declaration } $self->{client}->member_vars );

    return <<END_STRUCT
struct $self->{full_struct_sym} {
    $member_declarations
};
END_STRUCT
}

sub to_c_header {
    my $self          = shift;
    my $client        = $self->{client};
    my $cnick         = $self->{cnick};
    my @functions     = $client->functions;
    my @methods       = $client->methods;
    my @novel_methods = $client->novel_methods;
    my @inert_vars    = $client->inert_vars;
    my $vtable_var    = $self->vtable_var;
    my $struct_def    = $self->struct_definition;
    my $prefix        = $self->get_prefix;
    my $PREFIX        = $self->get_PREFIX;
    my $c_file_sym    = "C_" . uc( $client->full_struct_sym );

    # If class inherits from something, include the parent class's header.
    my $parent_include = "";
    if ( my $parent = $client->get_parent ) {
        $parent_include = $parent->include_h;
        $parent_include = qq|#include "$parent_include"|;
    }

    # Add a C function definition for each method and each function.
    my $sub_declarations = "";
    for my $sub ( @functions, @novel_methods ) {
        $sub_declarations
            .= Clownfish::Binding::Core::Function->func_declaration($sub)
            . "\n\n";
    }

    # Declare class (a.k.a. "inert") variables.
    my $inert_vars = "";
    for my $inert_var ( $client->inert_vars ) {
        $inert_vars .= "extern " . $inert_var->global_c . ";\n";
    }

    # Declare typedefs for novel methods, to ease casting.
    my $method_typedefs = '';
    for my $method (@novel_methods) {
        $method_typedefs
            .= Clownfish::Binding::Core::Method->typedef_dec($method) . "\n";
    }

    # Define method invocation syntax.
    my $method_defs = '';
    for my $method (@methods) {
        $method_defs .= Clownfish::Binding::Core::Method->method_def(
            method => $method,
            class  => $self->{client},
        ) . "\n";
    }

    # Declare the virtual table singleton object.
    my $vt_type       = $PREFIX . $self->vtable_type;
    my $vt            = "extern struct $vt_type $PREFIX${vtable_var}_vt;";
    my $vtable_object = "#define $PREFIX$vtable_var "
        . "((${prefix}VTable*)&$PREFIX${vtable_var}_vt)";
    my $num_methods = scalar @methods;

    # Declare Callback objects.
    my $callback_declarations = "";
    for my $method (@novel_methods) {
        next unless $method->public || $method->abstract;
        $callback_declarations
            .= Clownfish::Binding::Core::Method->callback_dec($method);
    }

    # Define short names.
    my $short_names = '';
    for my $function (@functions) {
        my $short_func_sym = $function->short_sym;
        my $full_func_sym  = $function->full_sym;
        $short_names .= "  #define $short_func_sym $full_func_sym\n";
    }
    for my $inert_var (@inert_vars) {
        my $short_name = "$self->{cnick}_" . $inert_var->micro_sym;
        $short_names .= "  #define $short_name $prefix$short_name\n";
    }
    if ( !$client->inert ) {
        for my $method (@novel_methods) {
            if ( !$method->isa("Clownfish::Method::Overridden") ) {
                my $short_typedef = $method->short_typedef;
                my $full_typedef  = $method->full_typedef;
                $short_names .= "  #define $short_typedef $full_typedef\n";
            }
            my $short_func_sym = $method->short_func_sym;
            my $full_func_sym  = $method->full_func_sym;
            $short_names .= "  #define $short_func_sym $full_func_sym\n";
        }
        for my $method (@methods) {
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

$inert_vars

$sub_declarations

#ifdef ${PREFIX}USE_SHORT_NAMES
$short_names
#endif /* ${PREFIX}USE_SHORT_NAMES */

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

$inert_vars

$sub_declarations
$callback_declarations

$method_typedefs

$method_defs

typedef struct $vt_type {
    ${prefix}VTable *vtable;
    kino_ref_t ref;
    ${prefix}VTable *parent;
    ${prefix}CharBuf *name;
    uint32_t flags;
    void *x;
    size_t obj_alloc_size;
    size_t vt_alloc_size;
    ${prefix}Callback **callbacks;
    kino_method_t methods[$num_methods];
} $vt_type;
$vt
$vtable_object

#ifdef ${PREFIX}USE_SHORT_NAMES
  #define $self->{struct_sym} $self->{full_struct_sym} 
  #define $vtable_var $PREFIX$vtable_var
$short_names
#endif /* ${PREFIX}USE_SHORT_NAMES */

END_STUFF
}

sub to_c {
    my $self   = shift;
    my $client = $self->{client};

    return $client->get_autocode if $client->inert;

    my $include_h      = $self->include_h;
    my $class_name_def = $self->name_var_definition;
    my $vtable_def     = $self->vtable_definition;
    my $autocode       = $client->get_autocode;
    my $offsets        = '';
    my $abstract_funcs = '';
    my $callback_funcs = '';
    my $callbacks      = '';
    my $prefix         = $self->get_prefix;
    my $PREFIX         = $self->get_PREFIX;
    my $vt_type        = $PREFIX . $self->vtable_type;
    my $meth_num       = 0;
    my @class_callbacks;

    # Prepare to identify novel methods.
    my %novel = map { ( $_->micro_sym => $_ ) } $client->novel_methods;

    for my $method ( $client->methods ) {
        my $var_name = $method->full_offset_sym( $self->{cnick} );

        # Create offset in bytes for the method from the top of the VTable
        # object.
        my $offset = "(offsetof($vt_type, methods)"
            . " + $meth_num * sizeof(kino_method_t))";
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

    # Create a NULL-terminated array of Callback vars.  Since C89 doesn't
    # allow us to initialize a pointer to an anonymous array inside a global
    # struct, we have to give it a real symbol and then store a pointer to
    # that symbol inside the VTable struct.
    my $callbacks_var = $PREFIX . $self->vtable_var . "_CALLBACKS";
    $callbacks .= "${prefix}Callback *$callbacks_var" . "[] = {\n    ";
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

=head2 get_prefix get_Prefix get_PREFIX

Forwards to the client's methods of the same name.

=head2 file_path

    # /path/to/Foo/Bar.c, if source class is Foo::Bar.
    my $path = $class->file_path( '/path/to', '.c' );

Provide an OS-specific path where a file relating to this class could be
found, by joining together the components of the "source class" name.

=head2 include_h

    print q|#include "| . $class->include_h . q|"|;

Return a relative path to a C header file, appropriately formatted for a
pound-include directive.

=head2 vtable_var

Return the name of the global VTable object for this class.

=head2 vtable_type

Return the C type specifier for this class's vtable.  Each vtable needs to
have its own type because each has a variable number of methods at the end of
the struct, and it's not possible to initialize a static struct with a
flexible array at the end under C89.

=head2 vtable_definition

Return C code defining the class's VTable.

=head2 struct_definition

Create the definition for the instantiable object struct.

=head2 callbacks_var

Return the name of the global Callbacks list for this class.

=head2 name_var 

The name of the global class name var for this class.

=head2 name_var_definition

C code defining the ZombieCharBuf which contains the class name for this
class.

=head2 to_c_header

Return the .h file which contains autogenerated C code defining the class's
interface:  all method invocation functions, etc...

=head2 to_c

Return the .c file which contains autogenerated C code necessary for the class
to function properly.

=cut
