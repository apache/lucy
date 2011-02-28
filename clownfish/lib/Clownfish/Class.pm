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

package Clownfish::Class;
use base qw( Clownfish::Symbol );
use Carp;
use Config;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Util qw(
    verify_args
    a_isa_b
);
use Clownfish::Dumpable;

END { __PACKAGE__->_clear_registry() }

our %create_PARAMS = (
    source_class      => undef,
    class_name        => undef,
    cnick             => undef,
    parent_class_name => undef,
    docucomment       => undef,
    inert             => undef,
    final             => undef,
    parcel            => undef,
    exposure          => 'parcel',
);

our %fetch_singleton_PARAMS = (
    parcel     => undef,
    class_name => undef,
);

sub fetch_singleton {
    my ( undef, %args ) = @_;
    verify_args( \%fetch_singleton_PARAMS, %args ) or confess $@;
    # Maybe prepend parcel prefix.
    my $parcel = $args{parcel};
    if ( defined $parcel ) {
        if ( !a_isa_b( $parcel, "Clownfish::Parcel" ) ) {
            $parcel = Clownfish::Parcel->singleton( name => $parcel );
        }
    }
    return _fetch_singleton( $parcel, $args{class_name} );
}

sub new { confess("The constructor for Clownfish::Class is create()") }

sub create {
    my ( $either, %args ) = @_;
    verify_args( \%create_PARAMS, %args ) or confess $@;
    $args{parcel} = Clownfish::Parcel->acquire( $args{parcel} );
    my $package = ref($either) || $either;
    return _create( $package, 
        @args{qw( parcel exposure class_name cnick micro_sym
        docucomment source_class parent_class_name final inert )} );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Class - An object representing a single class definition.

=head1 CONSTRUCTORS

Clownfish::Class objects are stored as quasi-singletons, one for each
unique parcel/class_name combination.

=head2 fetch_singleton 

    my $class = Clownfish::Class->fetch_singleton(
        parcel     => 'Crustacean',
        class_name => 'Crustacean::Lobster::LobsterClaw',
    );

Retrieve a Class, if one has already been created.

=head2 create

    my $class = Clownfish::Class->create(
        parcel     => 'Crustacean',                        # default: special
        class_name => 'Crustacean::Lobster::LobsterClaw',  # required
        cnick      => 'LobClaw',                           # default: special
        exposure   => 'public',                            # default: 'parcel'
        source_class      => undef,              # default: same as class_name
        parent_class_name => 'Crustacean::Claw', # default: undef
        inert             => undef,              # default: undef
        docucomment       => $documcom,          # default: undef,
    );

Create and register a quasi-singleton.  May only be called once for each
unique parcel/class_name combination.

=over

=item * B<parcel>, B<class_name>, B<cnick>, B<exposure> - see
L<Clownfish::Symbol>.

=item * B<source_class> - The name of the class that owns the file in which
this class was declared.  Should be "Foo" if "Foo::FooJr" is defined in
C<Foo.cfh>.

=item * B<parent_class_name> - The name of this class's parent class.  Needed
in order to establish the class hierarchy.

=item * B<inert> - Should be true if the class is inert, i.e. cannot be
instantiated.

=item * B<docucomment> - A Clownfish::DocuComment describing this Class.

=back

=head1 METHODS

=head2 get_cnick get_struct_sym get_parent_class_name get_source_class
get_docucomment get_parent get_autocode inert final

Accessors.

=head2 set_parent

    $class->set_parent($ancestor);

Set the parent class.

=head2 add_child

    $class->add_child($child_class);

Add a child class. 

=head2 add_method

    $class->add_method($method);

Add a Method to the class.  Valid only before grow_tree() is called.

=head2 add_function

    $class->add_function($function);

Add a Function to the class.  Valid only before grow_tree() is called.

=head2 add_member_var

    $class->add_member_var($var);

Add a member variable to the class.  Valid only before grow_tree() is called.

=head2 add_inert_var

    $class->add_inert_var($var);

Add an inert (class) variable to the class.  Valid only before grow_tree() is
called.

=head2 add_attribute

    $class->add_attribute($var, $value);

Add an arbitrary attribute to the class.

=head2 function 

    my $do_stuff_function = $class->function("do_stuff");

Return the inert Function object for the supplied C<micro_sym>, if any.

=head2 method

    my $pinch_method = $class->method("Pinch");

Return the Method object for the supplied C<micro_sym> / C<macro_sym>, if any.

=head2 novel_method

    my $pinch_method = $class->novel_method("Pinch");

Return a Method object if the Method corresponding to the supplied string is
novel.

=head2 children 

    my $child_classes = $class->children;

Return an array of all child classes.

=head2 functions

    my $functions = $class->functions;

Return an array of all (inert) functions.

=head2 methods

    my $methods = $class->methods;

Return an array of all methods.

=head2 inert_vars

    my $inert_vars = $class->inert_vars;

Return an array of all inert (shared, class) variables.

=head2 member_vars

    my $members = $class->member_vars;

Return an array of all member variables.

=head2 novel_methods

    my $novel_methods = $class->novel_methods;

Return an array of all novel methods.

=head2 novel_member_vars

    my $new_members = $class->novel_member_vars;

Return an array of all novel member variables.

=head2 grow_tree

    $class->grow_tree;

Bequeath all inherited methods and members to children.

=head2 tree_to_ladder

    my $ordered = $class->tree_to_ladder;

Return this class and all its child classes as an array, where all children
appear after their parent nodes.

=head2 include_h

    my $relative_path = $class->include_h;

Return a relative path to a C header file, appropriately formatted for a
pound-include directive.

=head2 append_autocode

    $class->append_autocode($code);

Append auxiliary C code.

=head2 short_vtable_var

The short name of the global VTable object for this class.

=head2 full_vtable_var

Fully qualified vtable variable name, including the parcel prefix.

=head2 full_vtable_type

The fully qualified C type specifier for this class's vtable, including the
parcel prefix.  Each vtable needs to have its own type because each has a
variable number of methods at the end of the struct, and it's not possible to
initialize a static struct with a flexible array at the end under C89.

=head2 full_struct_sym

Fully qualified struct symbol, including the parcel prefix.

=cut
