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

package Clownfish::CFC;
our $VERSION = '0.01';

END {
    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

use XSLoader;
BEGIN { XSLoader::load( 'Clownfish::CFC', '0.01' ) }

{
    package Clownfish::CFC::Util;
    use base qw( Exporter );
    use Scalar::Util qw( blessed );
    use Carp;
    use Fcntl;

    BEGIN {
        our @EXPORT_OK = qw(
            slurp_text
            current
            strip_c_comments
            verify_args
            a_isa_b
            write_if_changed
            trim_whitespace
            is_dir
            make_dir
            make_path
        );
    }

    # Verify that named parameters exist in a defaults hash.  Returns false
    # and sets $@ if a problem is detected.
    sub verify_args {
        my $defaults = shift;    # leave the rest of @_ intact

        # Verify that args came in pairs.
        if ( @_ % 2 ) {
            my ( $package, $filename, $line ) = caller(1);
            $@
                = "Parameter error: odd number of args at $filename line $line\n";
            return 0;
        }

        # Verify keys, ignore values.
        while (@_) {
            my ( $var, undef ) = ( shift, shift );
            next if exists $defaults->{$var};
            my ( $package, $filename, $line ) = caller(1);
            $@ = "Invalid parameter: '$var' at $filename line $line\n";
            return 0;
        }

        return 1;
    }

    sub a_isa_b {
        my ( $thing, $class ) = @_;
        return 0 unless blessed($thing);
        return $thing->isa($class);
    }
}

{
    package Clownfish::CFC::Base;
}

{
    package Clownfish::CFC::Model::CBlock;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = ( contents => undef, );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        confess("Missing required param 'contents'")
            unless defined $args{contents};
        return _new( $args{contents} );
    }
}

{
    package Clownfish::CFC::Model::Class;
    BEGIN { push our @ISA, 'Clownfish::CFC::Model::Symbol' }
    use Carp;
    use Config;
    use Clownfish::CFC::Util qw(
        verify_args
        a_isa_b
    );

    our %create_PARAMS = (
        file_spec         => undef,
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
            $parcel = Clownfish::CFC::Model::Parcel->acquire($parcel);
        }
        return _fetch_singleton( $parcel, $args{class_name} );
    }

    sub new {
        confess(
            "The constructor for Clownfish::CFC::Model::Class is create()");
    }

    sub create {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%create_PARAMS, %args ) or confess $@;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _create(
            @args{
                qw( parcel exposure class_name cnick micro_sym docucomment
                    file_spec parent_class_name final inert)
                }
        );
    }
}

{
    package Clownfish::CFC::Model::DocuComment;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
}

{
    package Clownfish::CFC::Dumpable;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
}

{
    package Clownfish::CFC::Model::File;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = ( spec => undef );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( $args{spec} );
    }
}

{
    package Clownfish::CFC::Model::FileSpec;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = (
        source_dir  => undef,
        path_part   => undef,
        is_included => 0,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( @args{ qw( source_dir path_part is_included ) } );
    }
}

{
    package Clownfish::CFC::Model::Function;
    BEGIN { push our @ISA, 'Clownfish::CFC::Model::Symbol' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args a_isa_b );

    my %new_PARAMS = (
        return_type => undef,
        class_name  => undef,
        class_cnick => undef,
        param_list  => undef,
        micro_sym   => undef,
        docucomment => undef,
        parcel      => undef,
        inline      => undef,
        exposure    => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        $args{inline} ||= 0;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new(
            @args{
                qw( parcel exposure class_name class_cnick micro_sym
                    return_type param_list docucomment inline )
                }
        );
    }
}

{
    package Clownfish::CFC::Model::Hierarchy;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args );

    our %new_PARAMS = (
        dest => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( @args{qw( dest )} );
    }
}

{
    package Clownfish::CFC::Model::Method;
    BEGIN { push our @ISA, 'Clownfish::CFC::Model::Function' }
    use Clownfish::CFC::Util qw( verify_args );
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
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        $args{abstract} ||= 0;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        $args{final} ||= 0;
        return _new(
            @args{
                qw( parcel exposure class_name class_cnick macro_sym
                    return_type param_list docucomment final abstract )
                }
        );
    }
}

{
    package Clownfish::CFC::Model::ParamList;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = ( variadic => undef, );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        my $variadic = delete $args{variadic} || 0;
        return _new($variadic);
    }
}

{
    package Clownfish::CFC::Model::Parcel;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Scalar::Util qw( blessed );
    use Carp;

    our %new_PARAMS = (
        name    => undef,
        cnick   => undef,
        version => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        return _new( @args{qw( name cnick version )} );
    }

    our %new_from_json_PARAMS = (
        json => undef,
    );

    sub new_from_json {
        my ( $either, %args ) = @_;
        verify_args( \%new_from_json_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        return _new_from_json( $args{json} );
    }

    our %new_from_file_PARAMS = (
        path => undef,
    );

    sub new_from_file {
        my ( $either, %args ) = @_;
        verify_args( \%new_from_file_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        return _new_from_file( $args{path} );
    }

#    $parcel = Clownfish::CFC::Model::Parcel->acquire($parcel_name_or_parcel_object);
#
# Aquire a parcel one way or another.  If the supplied argument is a
# Parcel, return it.  If it's not defined, return the default Parcel.  If
# it's a name, fetch an existing Parcel or register a new one.
    sub acquire {
        my ( undef, $thing ) = @_;
        if ( !defined $thing ) {
            return Clownfish::CFC::Model::Parcel->default_parcel;
        }
        elsif ( blessed($thing) ) {
            confess("Not a Clownfish::CFC::Model::Parcel")
                unless $thing->isa('Clownfish::CFC::Model::Parcel');
            return $thing;
        }
        else {
            my $parcel = Clownfish::CFC::Model::Parcel->fetch($thing);
            if ( !$parcel ) {
                $parcel
                    = Clownfish::CFC::Model::Parcel->new( name => $thing, );
                $parcel->register;
            }
            return $parcel;
        }
    }
}

{
    package Clownfish::CFC::Parser;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
}

{
    package Clownfish::CFC::Parser;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
}

{
    package Clownfish::CFC::Model::Symbol;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    my %new_PARAMS = (
        parcel      => undef,
        exposure    => undef,
        class_name  => undef,
        class_cnick => undef,
        micro_sym   => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new(
            @args{qw( parcel exposure class_name class_cnick micro_sym )} );
    }
}

{
    package Clownfish::CFC::Model::Type;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args a_isa_b );
    use Scalar::Util qw( blessed );
    use Carp;

    our %new_PARAMS = (
        const       => undef,
        specifier   => undef,
        indirection => undef,
        parcel      => undef,
        c_string    => undef,
        void        => undef,
        object      => undef,
        primitive   => undef,
        integer     => undef,
        floating    => undef,
        string_type => undef,
        va_list     => undef,
        arbitrary   => undef,
        composite   => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        my $package = ref($either) || $either;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;

        my $flags = 0;
        $flags |= CONST       if $args{const};
        $flags |= NULLABLE    if $args{nullable};
        $flags |= VOID        if $args{void};
        $flags |= OBJECT      if $args{object};
        $flags |= PRIMITIVE   if $args{primitive};
        $flags |= INTEGER     if $args{integer};
        $flags |= FLOATING    if $args{floating};
        $flags |= STRING_TYPE if $args{string_type};
        $flags |= VA_LIST     if $args{va_list};
        $flags |= ARBITRARY   if $args{arbitrary};
        $flags |= COMPOSITE   if $args{composite};

        my $parcel
            = $args{parcel}
            ? Clownfish::CFC::Model::Parcel->acquire( $args{parcel} )
            : $args{parcel};

        my $indirection = $args{indirection} || 0;
        my $specifier   = $args{specifier}   || '';
        my $c_string    = $args{c_string}    || '';

        return _new( $flags, $parcel, $specifier, $indirection, $c_string );
    }

    our %new_integer_PARAMS = (
        const     => undef,
        specifier => undef,
    );

    sub new_integer {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_integer_PARAMS, %args ) or confess $@;
        my $flags = 0;
        $flags |= CONST if $args{const};
        return _new_integer( $flags, $args{specifier} );
    }

    our %new_float_PARAMS = (
        const     => undef,
        specifier => undef,
    );

    sub new_float {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_float_PARAMS, %args ) or confess $@;
        my $flags = 0;
        $flags |= CONST if $args{const};
        return _new_float( $flags, $args{specifier} );
    }

    our %new_object_PARAMS = (
        const       => undef,
        specifier   => undef,
        indirection => 1,
        parcel      => undef,
        incremented => 0,
        decremented => 0,
        nullable    => 0,
    );

    sub new_object {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_object_PARAMS, %args ) or confess $@;
        my $flags = 0;
        $flags |= INCREMENTED if $args{incremented};
        $flags |= DECREMENTED if $args{decremented};
        $flags |= NULLABLE    if $args{nullable};
        $flags |= CONST       if $args{const};
        $args{indirection} = 1 unless defined $args{indirection};
        my $parcel = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        my $package = ref($either) || $either;
        confess("Missing required param 'specifier'")
            unless defined $args{specifier};
        return _new_object( $flags, $parcel, $args{specifier},
            $args{indirection} );
    }

    our %new_composite_PARAMS = (
        child       => undef,
        indirection => undef,
        array       => undef,
        nullable    => undef,
    );

    sub new_composite {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_composite_PARAMS, %args ) or confess $@;
        my $flags = 0;
        $flags |= NULLABLE if $args{nullable};
        my $indirection = $args{indirection} || 0;
        my $array = defined $args{array} ? $args{array} : "";
        return _new_composite( $flags, $args{child}, $indirection, $array );
    }

    our %new_void_PARAMS = ( const => undef, );

    sub new_void {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_void_PARAMS, %args ) or confess $@;
        return _new_void( !!$args{const} );
    }

    sub new_va_list {
        my $either = shift;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( {}, @_ ) or confess $@;
        return _new_va_list();
    }

    our %new_arbitrary_PARAMS = (
        parcel    => undef,
        specifier => undef,
    );

    sub new_arbitrary {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_arbitrary_PARAMS, %args ) or confess $@;
        my $parcel = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new_arbitrary( $parcel, $args{specifier} );
    }
}

{
    package Clownfish::CFC::Model::Variable;
    BEGIN { push our @ISA, 'Clownfish::CFC::Model::Symbol' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = (
        type        => undef,
        micro_sym   => undef,
        parcel      => undef,
        exposure    => 'local',
        class_name  => undef,
        class_cnick => undef,
        inert       => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        $args{exposure} ||= 'local';
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new(
            @args{
                qw( parcel exposure class_name class_cnick micro_sym type inert )
                }
        );
    }
}

{
    package Clownfish::CFC::Model::Version;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = (
        vstring => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess "no subclassing allowed" unless $either eq __PACKAGE__;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( $args{vstring} );
    }
}

{
    package Clownfish::CFC::Binding::Core;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = (
        hierarchy => undef,
        header    => undef,
        footer    => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( @args{qw( hierarchy header footer )} );
    }
}

{
    package Clownfish::CFC::Binding::Core::Class;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( a_isa_b verify_args );
    use Carp;

    our %new_PARAMS = ( client => undef, );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        return _new( $args{client} );
    }
}

{
    package Clownfish::CFC::Binding::Core::File;
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    my %write_h_PARAMS = (
        file   => undef,
        dest   => undef,
        header => undef,
        footer => undef,
    );

    sub write_h {
        my ( undef, %args ) = @_;
        verify_args( \%write_h_PARAMS, %args ) or confess $@;
        _write_h( @args{qw( file dest header footer )} );
    }
}

{
    package Clownfish::CFC::Binding::Core::Method;

    sub method_def {
        my ( undef, %args ) = @_;
        return _method_def( @args{qw( method class )} );
    }

    sub callback_obj_def {
        my ( undef, %args ) = @_;
        return _callback_obj_def( @args{qw( method offset )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args a_isa_b );

    our %new_PARAMS = (
        parcel     => undef,
        hierarchy  => undef,
        lib_dir    => undef,
        boot_class => undef,
        header     => undef,
        footer     => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new(
            @args{qw( parcel hierarchy lib_dir boot_class header footer )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl::Class;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args );

    our %new_PARAMS = (
        parcel     => undef,
        class_name => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        verify_args( \%new_PARAMS, %args ) or confess $@;
        $args{parcel}
            = Clownfish::CFC::Model::Parcel->acquire( $args{parcel} );
        return _new( @args{qw( parcel class_name )} );
    }

    our %bind_method_PARAMS = (
        alias  => undef,
        method => undef,
    );

    sub bind_method {
        my ( $self, %args ) = @_;
        verify_args( \%bind_method_PARAMS, %args ) or confess $@;
        _bind_method( $self, @args{qw( alias method )} );
    }

    our %bind_constructor_PARAMS = (
        alias       => undef,
        initializer => undef,
    );

    sub bind_constructor {
        my ( $self, %args ) = @_;
        verify_args( \%bind_constructor_PARAMS, %args ) or confess $@;
        _bind_constructor( $self, @args{qw( alias initializer )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl::Constructor;
    BEGIN { push our @ISA, 'Clownfish::CFC::Binding::Perl::Subroutine' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args );

    our %new_PARAMS = (
        class       => undef,
        alias       => undef,
        initializer => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess $@ unless verify_args( \%new_PARAMS, %args );
        return _new( @args{qw( class alias initializer )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl::Method;
    BEGIN { push our @ISA, 'Clownfish::CFC::Binding::Perl::Subroutine' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    our %new_PARAMS = (
        method => undef,
        alias  => undef,
    );

    sub new {
        my ( $either, %args ) = @_;
        confess $@ unless verify_args( \%new_PARAMS, %args );
        return _new( @args{qw( method alias )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl::Pod;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Clownfish::CFC::Util qw( verify_args );
    use Carp;

    my %add_method_PARAMS = (
        alias  => undef,
        method => undef,
        sample => undef,
        pod    => undef,
    );

    sub add_method {
        my ( $self, %args ) = @_;
        verify_args( \%add_method_PARAMS, %args ) or confess $@;
        _add_method( $self, @args{qw( alias method sample pod )} );
    }

    my %add_constructor_PARAMS = (
        alias       => undef,
        initializer => undef,
        sample      => undef,
        pod         => undef,
    );

    sub add_constructor {
        my ( $self, %args ) = @_;
        verify_args( \%add_constructor_PARAMS, %args ) or confess $@;
        _add_constructor( $self, @args{qw( alias initializer sample pod )} );
    }
}

{
    package Clownfish::CFC::Binding::Perl::Subroutine;
    BEGIN { push our @ISA, 'Clownfish::CFC::Base' }
    use Carp;
    use Clownfish::CFC::Util qw( verify_args );

    sub xsub_def { confess "Abstract method" }
}

{
    package Clownfish::CFC::Binding::Perl::TypeMap;
    use base qw( Exporter );

    our @EXPORT_OK = qw( from_perl to_perl );

    sub write_xs_typemap {
        my ( undef, %args ) = @_;
        _write_xs_typemap( $args{hierarchy} );
    }
}

1;

=head1 NAME

Clownfish::CFC - Clownfish compiler.

=head1 PRIVATE API

CFC is an Apache Lucy implementation detail.  This documentation is partial --
enough for the curious hacker, but not a full API.

=head1 SYNOPSIS

    use Clownfish::CFC::Model::Hierarchy;
    use Clownfish::CFC::Binding::Core;

    # Compile all .cfh files in $cf_source into 'autogen'.
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(
        source => $cf_source,
        dest   => 'autogen',
    );  
    $hierarchy->build;
    my $core_binding = Clownfish::CFC::Binding::Core->new(
        hierarchy => $hierarchy,
        dest      => 'autogen',
        header    => $license_header,
        footer    => '', 
    );  
    $core_binding->write_all_modified;

=head1 COPYRIGHT 
 
Clownfish is distributed under the Apache License, Version 2.0, as 
described in the file C<LICENSE> included with the distribution. 

=cut

