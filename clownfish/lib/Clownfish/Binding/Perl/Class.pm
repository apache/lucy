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

package Clownfish::Binding::Perl::Class;
use base qw( Clownfish::Base );
use Clownfish::Util qw( verify_args );
use Clownfish::Binding::Perl::Pod;
use Carp;

our %registry;
sub registry { \%registry }

our %register_PARAMS = (
    parcel            => undef,
    class_name        => undef,
    bind_methods      => undef,
    bind_constructors => undef,
    make_pod          => undef,
    xs_code           => undef,
    client            => undef,
);

our %bind_methods;
our %bind_constructors;
our %make_pod;
our %pod_spec;

sub register {
    my ( $either, %args ) = @_;
    verify_args( \%register_PARAMS, %args ) or confess $@;
    $args{parcel} = Clownfish::Parcel->acquire( $args{parcel} );

    # Validate.
    confess("Missing required param 'class_name'")
        unless $args{class_name};
    confess("$args{class_name} already registered")
        if exists $registry{ $args{class_name} };

    # Retrieve Clownfish::Class client, if it will be needed.
    my $client;
    if (   $args{bind_methods}
        || $args{bind_constructors}
        || $args{make_pod} )
    {
        $args{client} = Clownfish::Class->fetch_singleton(
            parcel     => $args{parcel},
            class_name => $args{class_name},
        );
        confess("Can't fetch singleton for $args{class_name}")
            unless $args{client};
    }

    # Create object.
    my $self = _new( @args{qw( parcel class_name client xs_code )} );
    $bind_methods{$self}      = $args{bind_methods};
    $bind_constructors{$self} = $args{bind_constructors};
    if ( $args{make_pod} ) {
        $make_pod{$self} = $args{make_pod};
        $pod_spec{$self}
            = Clownfish::Binding::Perl::Pod->new( %{ $args{make_pod} } );
    }

    # Add to registry.
    $registry{ $args{class_name} } = $self;

    return $self;
}

sub DESTROY {
    my $self = shift;
    delete $bind_methods{$self};
    delete $bind_constructors{$self};
    delete $make_pod{$self};
    delete $pod_spec{$self};
    _destroy($self);
}

sub get_bind_methods      { $bind_methods{ +shift } }
sub get_bind_constructors { $bind_constructors{ +shift } }
sub get_make_pod          { $make_pod{ +shift } }
sub get_pod_spec          { $pod_spec{ +shift } }

sub constructor_bindings {
    my $self  = shift;
    my @bound = map {
        my $xsub = Clownfish::Binding::Perl::Constructor->new(
            class => $self->get_client,
            alias => $_,
        );
    } @{ $self->get_bind_constructors };
    return @bound;
}

sub method_bindings {
    my $self       = shift;
    my $client     = $self->get_client;
    my $meth_list  = $self->get_bind_methods;
    my $class_name = $self->get_class_name;
    my @bound;

    # Assemble a list of methods to be bound for this class.
    my %meth_to_bind;
    for my $meth_namespec (@$meth_list) {
        my ( $alias, $name )
            = $meth_namespec =~ /^(.*?)\|(.*)$/
            ? ( $1, $2 )
            : ( lc($meth_namespec), $meth_namespec );
        $meth_to_bind{$name} = { alias => $alias };
    }

    # Iterate over all this class's methods, stopping to bind each one that
    # was spec'd.
    for my $method ( @{ $client->methods } ) {
        my $meth_name = $method->get_macro_sym;
        my $bind_args = delete $meth_to_bind{$meth_name};
        next unless $bind_args;

        # Safety checks against excess binding code or private methods.
        if ( !$method->novel ) {
            confess(  "Binding spec'd for method '$meth_name' in class "
                    . "$class_name, but it's overridden and "
                    . "should be bound via the parent class" );
        }
        elsif ( $method->private ) {
            confess(  "Binding spec'd for method '$meth_name' in class "
                    . "$class_name, but it's private" );
        }

        # Create an XSub binding for each override.  Each of these directly
        # calls the implementing function, rather than invokes the method on
        # the object using VTable method dispatch.  Doing things this way
        # allows SUPER:: invocations from Perl-space to work properly.
        for my $descendant ( @{ $client->tree_to_ladder } ) {  # includes self
            my $real_method = $descendant->novel_method( lc($meth_name) );
            next unless $real_method;

            # Create the binding, add it to the array.
            my $method_binding = Clownfish::Binding::Perl::Method->new(
                method => $real_method,
                %$bind_args,
            );
            push @bound, $method_binding;
        }
    }

    # Verify that we processed all methods.
    my @leftover_meths = keys %meth_to_bind;
    confess("Leftover for $class_name: '@leftover_meths'")
        if @leftover_meths;

    return @bound;
}

sub create_pod {
    my $self       = shift;
    my $pod_args   = $self->get_make_pod or return;
    my $pod_spec   = $self->get_pod_spec;
    my $class_name = $self->get_class_name;
    my $class      = $self->get_client or die "No client for $class_name";
    my $docucom    = $class->get_docucomment;
    confess("No DocuComment for '$class_name'") unless $docucom;
    my $brief       = $docucom->get_brief;
    my $description = $pod_spec->_perlify_doc_text( $pod_spec->get_description
            || $docucom->get_long );

    # Create SYNOPSIS.
    my $synopsis_pod = $pod_spec->get_synopsis;
    if ($synopsis_pod) {
        $synopsis_pod = qq|=head1 SYNOPSIS\n\n$synopsis_pod\n|;
    }

    # Create CONSTRUCTORS.
    my $constructor_pod = $pod_spec->constructors_pod($class);

    # Create METHODS, possibly including an ABSTRACT METHODS section.
    my $methods_pod = $pod_spec->methods_pod($class);

    # Build an INHERITANCE section describing class ancestry.
    my $child = $class;
    my @ancestors;
    while ( defined( my $parent = $child->get_parent ) ) {
        push @ancestors, $parent;
        $child = $parent;
    }
    my $inheritance_pod = "";
    if (@ancestors) {
        $inheritance_pod = "=head1 INHERITANCE\n\n";
        $inheritance_pod .= $class->get_class_name;
        for my $ancestor (@ancestors) {
            my $class_name = $ancestor->get_class_name;           
            if ( $registry{$class_name} ) {
                $inheritance_pod .= " isa L<$class_name>";
            } 
            else {
                $inheritance_pod .= " isa $class_name";
            }
        }
        $inheritance_pod .= ".\n";
    }

    # Put it all together.
    my $pod = <<END_POD;
# Auto-generated file -- DO NOT EDIT!!!!!

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

==head1 NAME

$class_name - $brief

$synopsis_pod

==head1 DESCRIPTION

$description

$constructor_pod

$methods_pod

$inheritance_pod

==cut

END_POD

    # Kill off stupid hack which allows us to embed pod in this file without
    # messing up what you see when you perldoc it.
    $pod =~ s/^==/=/gm;

    return $pod;
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl::Class - Generate Perl binding code for a
Clownfish::Class.

=head1 CLASS METHODS

=head1 register

    Clownfish::Binding::Perl::Class->register(
        parcel       => 'MyProject' ,                         # required
        class_name   => 'Foo::FooJr',                         # required
        bind_methods => [qw( Do_Stuff _get_foo|Get_Foo )],    # default: undef
        bind_constructors => [qw( new _new2|init2 )],         # default: undef
        make_pod          => [qw( get_foo )],                 # default: undef
        xs_code           => undef,                           # default: undef
    );

Create a new class binding and lodge it in the registry.  May only be called
once for each unique class name, and must be called after all classes have
been parsed (via Clownfish::Hierarchy's build()).

=over

=item * B<parcel> - A L<Clownfish::Parcel> or parcel name.

=item * B<class_name> - The name of the class to be registered.

=item * B<xs_code> - Raw XS code to be included in the final .xs file
generated by Clownfish::Binding::Perl. The XS directives PACKAGE and
MODULE should be specified.

=item * B<bind_methods> - An array of names for novel methods for which XS
bindings should be auto-generated, supplied using Clownfish's C<Macro_Name>
method-naming convention.  The Perl subroutine name will be derived by
lowercasing C<Method_Name> to C<method_name>, but this can be overridden by
prepending an alias and a pipe: e.g. C<_get_foo|Get_Foo>.

=item * B<bind_constructors> - An array of constructor names.  The default
implementing function is the class's C<init> function, unless it is overridden
using a pipe-separated string: C<_new2|init2> would create a Perl subroutine
"_new2" which would invoke C<myproj_FooJr_init2>.

=item * B<make_pod> - A specification for generating POD.  TODO: document this
spec, or break it up into multiple methods.  (For now, just see examples from
the source code.)

=back

=head1 registry

    my $registry = Clownfish::Binding::Perl::Class->registry;
    while ( my $class_name, $class_binding ) = each %$registry ) {
        ...
    }

Return the hash registry used by register().  The keys are class names, and
the values are Clownfish::Binding::Perl::Class objects.

=head1 OBJECT METHODS

=head2 get_class_name get_bind_methods get_bind_methods get_pod_spec
get_xs_code get_client

Accessors.  C<get_client> retrieves the Clownfish::Class module to be
bound.

=head2 constructor_bindings

    my @ctor_bindings = $class_binding->constructor_bindings;

Return a list of Clownfish::Binding::Perl::Constructor objects created as
per the C<bind_constructors> spec.

=head2 method_bindings

    my @method_bindings = $class_binding->method_bindings;

Return a list of Clownfish::Binding::Perl::Method objects created as per
the C<bind_methods> spec.

=head2 create_pod

    my $pod = $class_binding->create_pod;

Auto-generate POD according to the make_pod spec, if such a spec was supplied.

=cut
