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
$full_method_sym($params) {
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

sub callback_obj_def {
    my ( undef, %args ) = @_;
    return _callback_obj_def( @args{qw( method offset )} );
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
method" error at runtime, for methods which are declared as "abstract" in a
Clownfish header file.

=cut
