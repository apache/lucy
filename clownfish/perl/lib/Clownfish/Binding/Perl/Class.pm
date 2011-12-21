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
use Clownfish::Util qw( verify_args );
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

sub register {
    my $either = shift;
    verify_args( \%register_PARAMS, @_ ) or confess $@;
    my $self = bless { %register_PARAMS, @_, }, ref($either) || $either;

    # Validate.
    confess("Missing required param 'class_name'")
        unless $self->{class_name};
    confess("$self->{class_name} already registered")
        if exists $registry{ $self->{class_name} };

    # Retrieve Clownfish::Class client, if it will be needed.
    if (   $self->{bind_methods}
        || $self->{bind_constructors}
        || $self->{make_pod} )
    {
        $self->{client} = Clownfish::Class->fetch_singleton(
            parcel     => $self->{parcel},
            class_name => $self->{class_name},
        );
        confess("Can't fetch singleton for $self->{class_name}")
            unless $self->{client};
    }

    # Add to registry.
    $registry{ $self->{class_name} } = $self;

    return $self;
}

sub registered { [ values %registry ] }
sub singleton  { $registry{ $_[1] } }

sub get_class_name        { shift->{class_name} }
sub get_bind_methods      { shift->{bind_methods} }
sub get_bind_constructors { shift->{bind_constructors} }
sub get_make_pod          { shift->{make_pod} }
sub get_client            { shift->{client} }
sub get_xs_code           { shift->{xs_code} }

sub constructor_bindings {
    my $self  = shift;
    my @bound = map {
        my $xsub = Clownfish::Binding::Perl::Constructor->new(
            class => $self->{client},
            alias => $_,
        );
    } @{ $self->{bind_constructors} };
    return @bound;
}

sub method_bindings {
    my $self      = shift;
    my $client    = $self->{client};
    my $meth_list = $self->{bind_methods};
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
                    . "$self->{class_name}, but it's overridden and "
                    . "should be bound via the parent class" );
        }
        elsif ( $method->private ) {
            confess(  "Binding spec'd for method '$meth_name' in class "
                    . "$self->{class_name}, but it's private" );
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
    confess("Leftover for $self->{class_name}: '@leftover_meths'")
        if @leftover_meths;

    return @bound;
}

sub _gen_subroutine_pod {
    my ( $self, %args ) = @_;
    my ( $func, $sub_name, $class, $code_sample, $class_name )
        = @args{qw( func name class sample class_name )};
    my $param_list = $func->get_param_list;
    my $args       = "";
    my $num_vars   = $param_list->num_vars;

    # Only allow "public" subs to be exposed as part of the public API.
    confess("$class_name->$sub_name is not public") unless $func->public;

    # Get documentation, which may be inherited.
    my $docucom = $func->get_docucomment;
    if ( !$docucom ) {
        my $micro_sym = $func->micro_sym;
        my $parent    = $class;
        while ( $parent = $parent->get_parent ) {
            my $parent_func = $parent->method($micro_sym);
            last unless $parent_func;
            $docucom = $parent_func->get_docucomment;
            last if $docucom;
        }
    }
    confess("No DocuComment for '$sub_name' in '$class_name'")
        unless $docucom;

    # Build string summarizing arguments to use in header.
    if ( $num_vars > 2 or ( $args{is_constructor} && $num_vars > 1 ) ) {
        $args = " I<[labeled params]> ";
    }
    elsif ( $param_list->num_vars ) {
        $args = $func->get_param_list->name_list;
        $args =~ s/self.*?(?:,\s*|$)//;    # kill self param
    }

    # Add code sample.
    my $pod = "=head2 $sub_name($args)\n\n";
    if ( defined($code_sample) && length($code_sample) ) {
        $pod .= "$code_sample\n";
    }

    # Incorporate "description" text from DocuComment.
    if ( my $long_doc = $docucom->get_description ) {
        $pod .= _perlify_doc_text($long_doc) . "\n\n";
    }

    # Add params in a list.
    my $param_names = $docucom->get_param_names;
    my $param_docs  = $docucom->get_param_docs;
    if (@$param_names) {
        $pod .= "=over\n\n";
        for ( my $i = 0; $i <= $#$param_names; $i++ ) {
            $pod .= "=item *\n\n";
            $pod .= "B<$param_names->[$i]> - $param_docs->[$i]\n\n";
        }
        $pod .= "=back\n\n";
    }

    # Add return value description, if any.
    if ( defined( my $retval = $docucom->get_retval ) ) {
        $pod .= "Returns: $retval\n\n";
    }

    return $pod;
}

sub create_pod {
    my $self     = shift;
    my $pod_args = $self->{make_pod} or return;
    my $class    = $self->{client} or die "No client for $self->{class_name}";
    my $class_name = $class->get_class_name;
    my $docucom    = $class->get_docucomment;
    confess("No DocuComment for '$class_name'") unless $docucom;
    my $brief = $docucom->get_brief;
    my $description
        = _perlify_doc_text( $pod_args->{description} || $docucom->get_long );

    # Create SYNOPSIS.
    my $synopsis_pod = '';
    if ( defined $pod_args->{synopsis} ) {
        $synopsis_pod = qq|=head1 SYNOPSIS\n\n$pod_args->{synopsis}\n|;
    }

    # Create CONSTRUCTORS.
    my $constructor_pod = "";
    my $constructors = $pod_args->{constructors} || [];
    if ( defined $pod_args->{constructor} ) {
        push @$constructors, $pod_args->{constructor};
    }
    if (@$constructors) {
        $constructor_pod = "=head1 CONSTRUCTORS\n\n";
        for my $spec (@$constructors) {
            if ( !ref $spec ) {
                $constructor_pod .= _perlify_doc_text($spec);
            }
            else {
                my $func_name   = $spec->{func} || 'init';
                my $init_func   = $class->function($func_name);
                my $ctor_name   = $spec->{name} || 'new';
                my $code_sample = $spec->{sample};
                $constructor_pod .= _perlify_doc_text(
                    $self->_gen_subroutine_pod(
                        func           => $init_func,
                        name           => $ctor_name,
                        sample         => $code_sample,
                        class          => $class,
                        class_name     => $class_name,
                        is_constructor => 1,
                    )
                );
            }
        }
    }

    # Create METHODS, possibly including an ABSTRACT METHODS section.
    my @method_docs;
    my $methods_pod = "";
    my @abstract_method_docs;
    my $abstract_methods_pod = "";
    for my $spec ( @{ $pod_args->{methods} } ) {
        my $meth_name = ref($spec) ? $spec->{name} : $spec;
        my $method = $class->method($meth_name);
        confess("Can't find method '$meth_name' in class '$class_name'")
            unless $method;
        my $method_pod;
        if ( ref($spec) ) {
            $method_pod = $spec->{pod};
        }
        else {
            $method_pod = $self->_gen_subroutine_pod(
                func       => $method,
                name       => $meth_name,
                sample     => '',
                class      => $class,
                class_name => $class_name
            );
        }
        if ( $method->abstract ) {
            push @abstract_method_docs, _perlify_doc_text($method_pod);
        }
        else {
            push @method_docs, _perlify_doc_text($method_pod);
        }
    }
    if (@method_docs) {
        $methods_pod = join( "", "=head1 METHODS\n\n", @method_docs );
    }
    if (@abstract_method_docs) {
        $abstract_methods_pod = join( "", "=head1 ABSTRACT METHODS\n\n",
            @abstract_method_docs );
    }

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

$abstract_methods_pod

$methods_pod

$inheritance_pod

==cut

END_POD

    # Kill off stupid hack which allows us to embed pod in this file without
    # messing up what you see when you perldoc it.
    $pod =~ s/^==/=/gm;

    return $pod;
}

sub _perlify_doc_text {
    my $documentation = shift;

    # Remove double-equals hack needed to fool perldoc, PAUSE, etc. :P
    $documentation =~ s/^==/=/mg;

    # Change <code>foo</code> to C<< foo >>.
    $documentation =~ s#<code>(.*?)</code>#C<< $1 >>#gsm;

    # Lowercase all method names: Open_In() => open_in()
    $documentation
        =~ s/([A-Z][A-Za-z0-9]*(?:_[A-Z][A-Za-z0-9]*)*\(\))/\L$1\E/gsm;

    # Change all instances of NULL to 'undef'
    $documentation =~ s/NULL/undef/g;

    # Change "Err_error" to "Lucy->error".
    $documentation =~ s/Err_error/Lucy->error/g;

    return $documentation;
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

=head2 get_class_name get_bind_methods get_bind_methods get_make_pod
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

