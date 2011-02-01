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

package Clownfish::Method;
use base qw( Clownfish::Function );
use Clownfish::Util qw( verify_args );
use Carp;

my %new_PARAMS = (
    return_type => undef,
    class_name  => undef,
    class_cnick => undef,
    param_list  => undef,
    macro_sym   => undef,
    docucomment => undef,
    parcel      => undef,
    abstract    => undef,
    final       => undef,
    exposure    => 'parcel',
    # Private, used only by finalize().
    novel         => undef,
    short_typedef => undef,
);

sub new {
    my ( $class, %args ) = @_;
    verify_args( \%new_PARAMS, %args ) or confess $@;
    my $abstract      = delete $args{abstract};
    my $final         = delete $args{final};
    my $macro_sym     = delete $args{macro_sym};
    my $novel         = delete $args{novel};
    my $short_typedef = delete $args{short_typedef};

    # Validate macro_sym, derive micro_sym.
    confess("macro_sym is required") unless defined $macro_sym;
    confess("Invalid macro_sym: '$macro_sym'")
        unless $macro_sym =~ /^[A-Z][A-Za-z0-9]*(?:_[A-Z0-9][A-Za-z0-9]*)*$/;
    $args{micro_sym} = lc($macro_sym);

    # Create self, add in novel member vars.
    my $self = $class->SUPER::new(%args);
    $self->{macro_sym} = $macro_sym;
    $self->{abstract}  = $abstract;
    $self->{final}     = $final;

    # Assume that this method is novel until we discover when applying
    # inheritance that it was overridden.
    $self->{novel} = defined $novel ? $novel : 1;

    # Verify that the first element in the arg list is a self.
    my $args = $self->get_param_list->get_variables;
    confess "Not enough args" unless @$args;
    my $specifier  = $args->[0]->get_type->get_specifier;
    my $class_name = $self->get_class_name;
    my ($struct_sym) = $class_name =~ /(\w+)$/;
    confess "First arg type doesn't match class: $class_name $specifier"
        unless $specifier eq $self->get_prefix . $struct_sym;

    # Cache typedef.
    $self->{short_typedef}
        = defined $short_typedef ? $short_typedef : $self->short_sym . "_t";

    return $self;
}

sub abstract      { shift->{abstract} }
sub novel         { shift->{novel} }
sub final         { shift->{final} }
sub get_macro_sym { shift->{macro_sym} }

sub self_type { shift->get_param_list->get_variables->[0]->get_type }

sub short_method_sym {
    my ( $self, $invoker ) = @_;
    confess("Missing invoker") unless $invoker;
    return $invoker . '_' . $self->get_macro_sym;
}

sub full_method_sym {
    my ( $self, $invoker ) = @_;
    return $self->get_Prefix . $self->short_method_sym($invoker);
}

# The name of the variable which stores the method's vtable offset.
sub full_offset_sym {
    my ( $self, $invoker ) = @_;
    confess("Missing invoker") unless $invoker;
    return $self->full_method_sym($invoker) . '_OFFSET';
}

sub full_callback_sym { shift->full_func_sym . "_CALLBACK" }
sub full_override_sym { shift->full_func_sym . "_OVERRIDE" }

sub short_typedef { shift->{short_typedef} }
sub full_typedef {
    my $self = shift;
    return $self->get_prefix . $self->short_typedef;
}

sub override {
    my ( $self, $orig ) = @_;

    # Check that the override attempt is legal.
    if ( $orig->final ) {
        my $orig_micro_sym = $orig->micro_sym;
        my $orig_class     = $orig->get_class_name;
        my $class_name     = $self->get_class_name;
        confess(  "Attempt to override final method '$orig_micro_sym' "
                . " from $orig_class by $class_name" );
    }
    if ( !$self->compatible($orig) ) {
        my $func_name = $self->full_func_sym;
        my $orig_func = $orig->full_func_sym;
        confess("Non-matching signatures for $func_name and $orig_func");
    }

    # Mark the Method as no longer novel.
    $self->{novel} = 0;
}

sub compatible {
    my ( $self, $other ) = @_;
    return 0 unless $self->get_macro_sym eq $other->get_macro_sym;
    return 0 if ( $self->public xor $other->public );
    my $param_list       = $self->get_param_list;
    my $other_param_list = $other->get_param_list;
    my $arg_vars         = $param_list->get_variables;
    my $other_vars       = $other_param_list->get_variables;
    my $initial_vals     = $param_list->get_initial_values;
    my $other_vals       = $other_param_list->get_initial_values;
    return 0 unless @$arg_vars == @$other_vars;
    return 0 unless @$initial_vals == @$other_vals;

    # Validate initial values.
    for ( my $i = 1; $i <= $#$arg_vars; $i++ ) {
        return 0 unless $other_vars->[$i]->equals( $arg_vars->[$i] );
        my $val       = $initial_vals->[$i];
        my $other_val = $other_vals->[$i];
        if ( defined $val ) {
            return 0 unless defined $other_val;
            return 0 unless $val eq $other_val;
        }
        else {
            return 0 if defined $other_val;
        }
    }

    # Weak validation of return type to allow covariant object return types.
    my $return_type       = $self->get_return_type;
    my $other_return_type = $other->get_return_type;
    if ( $return_type->is_object ) {
        return 0 unless $return_type->similar($other_return_type);
    }
    else {
        return 0 unless $return_type->equals($other_return_type);
    }

    return 1;
}

sub finalize {
    my $self = shift;
    return $self->new(
        return_type   => $self->get_return_type,
        class_name    => $self->get_class_name,
        class_cnick   => $self->get_class_cnick,
        param_list    => $self->get_param_list,
        macro_sym     => $self->get_macro_sym,
        docucomment   => $self->get_docucomment,
        parcel        => $self->get_parcel,
        abstract      => $self->abstract,
        final         => $self->final,
        exposure      => $self->get_exposure,
        novel         => $self->novel,
        short_typedef => $self->short_typedef,
        final         => 1,
    );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Method - Metadata describing an instance method.

=head1 DESCRIPTION

Clownfish::Method is a specialized subclass of Clownfish::Function, with
the first argument required to be an Obj.

When compiling Clownfish code to C, Method objects generate all the code
that Function objects do, but also create symbols for indirect invocation via
VTable.

=head1 METHODS

=head2 new

    my $type = Clownfish::Method->new(
        parcel      => 'Crustacean',                       # default: special
        class_name  => 'Crustacean::Lobster::LobsterClaw', # required
        class_cnick => 'LobClaw',                          # default: special
        macro_sym   => 'Pinch',                            # required
        return_type => $void_type,                         # required
        param_list  => $param_list,                        # required
        exposure    => undef,                              # default: 'parcel'
        docucomment => $docucomment,                       # default: undef
        abstract    => undef,                              # default: undef
        final       => 1,                                  # default: undef
    );

=over

=item * B<param_list> - A Clownfish::ParamList.  The first element must be an
object of the class identified by C<class_name>.

=item * B<macro_sym> - The mixed case name which will be used when invoking the
method.

=item * B<abstract> - Indicate whether the method is abstract.

=item * B<final> - Indicate whether the method is final.

=item * B<parcel>, B<class_name>, B<class_cnick>, B<return_type>,
B<docucomment>, - see L<Clownfish::Function>.

=back

=head2 abstract final get_macro_sym 

Getters.

=head2 novel

Returns true if the method's class is the first in the inheritance hierarchy
in which the method was declared -- i.e. the method is neither inherited nor
overridden.

=head2 self_type

Return the L<Clownfish::Type> for C<self>.

=head2 short_method_sym

    # e.g. "LobClaw_Pinch"
    my $short_sym = $method->short_method_sym("LobClaw");

Returns the symbol used to invoke the method (minus the parcel Prefix).

=head2 full_method_sym

    # e.g. "Crust_LobClaw_Pinch"
    my $full_sym = $method->full_method_sym("LobClaw");

Returns the fully-qualified symbol used to invoke the method.

=head2 full_offset_sym

    # e.g. "Crust_LobClaw_Pinch_OFFSET"
    my $offset_sym = $method->full_offset_sym("LobClaw");

Returns the fully qualified name of the variable which stores the method's
vtable offset.

=head2 full_callback_sym

    # e.g. "crust_LobClaw_pinch_CALLBACK"
    my $callback_sym = $method->full_calback_sym("LobClaw");

Returns the fully qualified name of the variable which stores the method's
Callback object.

=head2 full_override_sym

    # e.g. "crust_LobClaw_pinch_OVERRIDE"
    my $override_func_sym = $method->full_override_sym("LobClaw");

Returns the fully qualified name of the function which implements the callback
to the host in the event that a host method has been defined which overrides
this method.

=head2 short_typedef

    # e.g. "Claw_pinch_t"
    my $short_typedef = $method->short_typedef;

Returns the typedef symbol for this method, which is derived from the class
nick of the first class in which the method was declared.

=head2 full_typedef

    # e.g. "crust_Claw_pinch_t"
    my $full_typedef = $method->full_typedef;

Returns the fully-qualified typedef symbol including parcel prefix.

=head2 override

    $method->override($method_being_overridden);

Let the Method know that it is overriding a method which was defined in a
parent class, and verify that the override is valid.

All methods start out believing that they are "novel", because we don't know
about inheritance until we build the hierarchy after all files have been
parsed.  override() is a way of going back and relabeling a method as
overridden when new information has become available: in this case, that a
parent class has defined a method with the same name.

=head2 finalize

    my $final_method = $method->finalize;

As with override, above, this is for going back and changing the nature of a
Method after new information has become available -- typically, when we
discover that the method has been inherited by a "final" class.

However, we don't modify the original Method as with override().  Inherited
Method objects are shared between parent and child classes; if a shared Method
object were to become final, it would interfere with its own inheritance.  So,
we make a copy, slightly modified to indicate that it is "final".

=head2 compatible

    confess("Can't override") unless $method->compatible($other);

Returns true if the methods have signatures and attributes which allow
one to override the other.

=cut
