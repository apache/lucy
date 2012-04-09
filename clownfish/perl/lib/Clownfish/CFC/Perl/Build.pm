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

package Clownfish::CFC::Perl::Build;
use base qw( Module::Build );
our $VERSION = '0.01';

use File::Spec::Functions qw( catdir catfile curdir updir abs2rel rel2abs );
use File::Path qw( mkpath );
use Config;
use Carp;

# Add a custom Module::Build hashref property to pass additional build
# parameters
if ( $Module::Build::VERSION <= 0.30 ) {
    __PACKAGE__->add_property( clownfish_params => {} );
}
else {
    # TODO: add sub for property check
    __PACKAGE__->add_property(
        'clownfish_params',
        default => {},
    );
}

=for Rationale

When the distribution tarball for the Perl bindings is built, core/, and any
other needed files/directories are copied into the perl/ directory within the
main source directory.  Then the distro is built from the contents of the
perl/ directory, leaving out all the files in ruby/, etc. However, during
development, the files are accessed from their original locations.

=cut

my $is_distro_not_devel = -e 'core';
my $base_dir = rel2abs( $is_distro_not_devel ? getcwd() : updir() );
my $CORE_SOURCE_DIR = catdir( $base_dir, 'core' );

my $AUTOGEN_DIR  = 'autogen';
my $LIB_DIR      = 'lib';
my $BUILDLIB_DIR = 'buildlib';

sub new {
    my $self = shift->SUPER::new( @_ );

    my $extra_ccflags = $self->extra_compiler_flags;
    if ( $self->config('gccversion') ) {
        push @$extra_ccflags, qw( -std=gnu99 -D_GNU_SOURCE );
    }
    elsif ( $self->config('cc') =~ /^cl\b/ ) {
        # Compile as C++ under MSVC.
        push @$extra_ccflags, qw(
            /TP -D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS
        );
    }
    $self->extra_compiler_flags(@$extra_ccflags);

    # TODO: use Charmonizer to determine whether pthreads are userland.
    if ( $Config{osname} =~ /openbsd/i && $Config{usethreads} ) {
        my $extra_ldflags = $self->extra_linker_flags;
        push @$extra_ldflags, '-lpthread';
        $self->extra_linker_flags(@$extra_ldflags);
    }

    my $include_dirs = $self->include_dirs;
    push( @$include_dirs,
        curdir(), # for ppport.h
        catfile( $AUTOGEN_DIR, 'include' ),
    );
    $self->include_dirs($include_dirs);

    my $cf_include = $self->clownfish_params('include');
    if ( !defined($cf_include) ) {
        $cf_include = [];
    }
    elsif ( !ref($cf_include) ) {
        $cf_include = [ $cf_include ];
    }
    $self->clownfish_params( include => $cf_include );

    my $autogen_header = $self->clownfish_params('autogen_header');
    if ( !defined($autogen_header) ) {
        $self->clownfish_params( autogen_header => <<'END_AUTOGEN' );
/***********************************************

 !!!! DO NOT EDIT !!!!

 This file was auto-generated by Build.PL.

 ***********************************************/

END_AUTOGEN
    }

    return $self;
}

sub cf_system_include_dirs {
    my $self_or_class = shift;

    my @include_dirs;
    for my $location ( qw( site vendor ) ) {
        my $install_dir = $Config{"install${location}arch"};
        my $include_dir = catdir( $install_dir, 'Clownfish', '_include' );
        next unless -d $include_dir;
        push(@include_dirs, $include_dir);
    }

    return @include_dirs;
}

sub cf_system_library_file {
    my ( $self_or_class, $module_name ) = @_;

    my @module_parts = split( '::', $module_name );
    my $class_name   = $module_parts[-1];

    for my $location ( qw( site vendor ) ) {
        my $install_dir = $Config{"install${location}arch"};
        my $lib_file = catfile(
            $install_dir, 'auto', @module_parts, "$class_name.$Config{dlext}",
        );
        return $lib_file if -f $lib_file;
    }

    die("No Clownfish library file found for module $module_name");
}

sub ACTION_copy_clownfish_includes {
    my $self = shift;
    # Copy .cfh files to blib/arch/Clownfish/_include
    my $cfh_filepaths = $self->rscan_dir( $CORE_SOURCE_DIR, qr/\.cfh$/ );
    my $inc_dir = catdir( $self->blib, 'arch', 'Clownfish', '_include' );
    for my $file (@$cfh_filepaths) {
        my $rel  = abs2rel( $file, $CORE_SOURCE_DIR );
        my $dest = catfile( $inc_dir, $rel );
        $self->copy_if_modified( from => $file, to => $dest );
    }
}

sub _compile_clownfish {
    my $self = shift;

    require Clownfish::CFC::Model::Hierarchy;
    require Clownfish::CFC::Binding::Perl;
    require Clownfish::CFC::Binding::Perl::Class;

    # Compile Clownfish.
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(
        dest => $AUTOGEN_DIR,
    );
    $hierarchy->add_source_dir($CORE_SOURCE_DIR);
    my $include_dirs = $self->clownfish_params('include');
    for my $include_dir (@$include_dirs) {
        $hierarchy->add_include_dir($include_dir);
    }
    $hierarchy->build;

    # Process all Binding classes in buildlib.
    my $pm_filepaths = $self->rscan_dir( $BUILDLIB_DIR, qr/\.pm$/ );
    for my $pm_filepath (@$pm_filepaths) {
        next unless $pm_filepath =~ /Binding/;
        require $pm_filepath;
        my $package_name = $pm_filepath;
        $package_name =~ s/buildlib\/(.*)\.pm$/$1/;
        $package_name =~ s/\//::/g;
        $package_name->bind_all;
    }

    my $module_name  = $self->module_name;
    my @module_parts = split( '::', $module_name );
    my $parcel       = $module_parts[-1];

    my $binding = Clownfish::CFC::Binding::Perl->new(
        parcel     => $parcel,
        hierarchy  => $hierarchy,
        lib_dir    => $LIB_DIR,
        boot_class => $module_name,
        header     => $self->clownfish_params('autogen_header'),
        footer     => '',
    );

    return ( $hierarchy, $binding );
}

sub ACTION_pod {
    my $self = shift;
    $self->dispatch("clownfish");
    $self->_write_pod(@_);
}

sub _write_pod {
    my ( $self, $binding ) = @_;
    if ( !$binding ) {
        ( undef, $binding ) = $self->_compile_clownfish;
    }
    print "Writing POD...\n";
    my $pod_files = $binding->write_pod;
    $self->add_to_cleanup($_) for @$pod_files;
}

sub ACTION_clownfish {
    my $self = shift;

    $self->add_to_cleanup($AUTOGEN_DIR);

    my @module_dir  = split( '::', $self->module_name );
    my $class_name  = pop(@module_dir);
    my $xs_filepath = catfile( $LIB_DIR, @module_dir, "$class_name.xs" );

    my $buildlib_pm_filepaths = $self->rscan_dir( $BUILDLIB_DIR, qr/\.pm$/ );
    my $cfh_filepaths = $self->rscan_dir( $CORE_SOURCE_DIR, qr/\.cfh$/ );

    # XXX joes thinks this is dubious
    # Don't bother parsing Clownfish files if everything's up to date.
    return
        if $self->up_to_date(
        [ @$cfh_filepaths, @$buildlib_pm_filepaths ],
        [ $xs_filepath,    $AUTOGEN_DIR, ]
        );

    # Write out all autogenerated files.
    print "Parsing Clownfish files...\n";
    my ( $hierarchy, $perl_binding ) = $self->_compile_clownfish;
    require Clownfish::CFC::Binding::Core;
    my $core_binding = Clownfish::CFC::Binding::Core->new(
        hierarchy => $hierarchy,
        header    => $self->clownfish_params('autogen_header'),
        footer    => '',
    );
    print "Writing Clownfish autogenerated files...\n";
    my $modified = $core_binding->write_all_modified;
    if ($modified) {
        unlink('typemap');
        print "Writing typemap...\n";
        $self->add_to_cleanup('typemap');
        $perl_binding->write_xs_typemap;
    }

    # Rewrite XS if either any .cfh files or relevant .pm files were modified.
    $modified ||=
        $self->up_to_date( \@$buildlib_pm_filepaths, $xs_filepath )
        ? 0
        : 1;

    if ($modified) {
        $self->add_to_cleanup($xs_filepath);
        $perl_binding->write_boot;
        $perl_binding->write_bindings;
        $self->_write_pod($perl_binding);
    }

    # Touch autogenerated files in case the modifications were inconsequential
    # and didn't trigger a rewrite, so that we won't have to check them again
    # next pass.
    if (!$self->up_to_date(
            [ @$cfh_filepaths, @$buildlib_pm_filepaths ], $xs_filepath
        )
        )
    {
        utime( time, time, $xs_filepath );    # touch
    }
    if (!$self->up_to_date(
            [ @$cfh_filepaths, @$buildlib_pm_filepaths ], $AUTOGEN_DIR
        )
        )
    {
        utime( time, time, $AUTOGEN_DIR );    # touch
    }
}

# Write ppport.h, which supplies some XS routines not found in older Perls and
# allows us to use more up-to-date XS API while still supporting Perls back to
# 5.8.3.
#
# The Devel::PPPort docs recommend that we distribute ppport.h rather than
# require Devel::PPPort itself, but ppport.h isn't compatible with the Apache
# license.
sub ACTION_ppport {
    my $self = shift;
    if ( !-e 'ppport.h' ) {
        require Devel::PPPort;
        $self->add_to_cleanup('ppport.h');
        Devel::PPPort::WriteFile();
    }
}

sub ACTION_compile_custom_xs {
    my $self = shift;

    $self->dispatch('ppport');

    require ExtUtils::CBuilder;
    require ExtUtils::ParseXS;

    my $module_name  = $self->module_name;
    my @module_parts = split( '::', $module_name );
    my @module_dir   = @module_parts;
    my $class_name   = pop(@module_dir);

    my $cbuilder = ExtUtils::CBuilder->new( config => $self->config );
    my $libdir   = catdir( $LIB_DIR, @module_dir );
    my $archdir  = catdir( $self->blib, 'arch', 'auto', @module_parts );
    mkpath( $archdir, 0, 0777 ) unless -d $archdir;
    my @objects;

    # Compile C source files.
    my $autogen_source_dir = catfile( $AUTOGEN_DIR, 'source' );
    my $c_files = [];
    push @$c_files, @{ $self->rscan_dir( $CORE_SOURCE_DIR,    qr/\.c$/ ) };
    push @$c_files, @{ $self->rscan_dir( $autogen_source_dir, qr/\.c$/ ) };
    my $c_sources = $self->clownfish_params('extra_c_sources') || [];
    for my $c_source (@$c_sources) {
        if ( -d $c_source ) {
            push @$c_files, @{ $self->rscan_dir( $c_source, qr/\.c$/ ) };
        }
        elsif ( $c_source =~ /\.c$/ ) {
            push @$c_files, $c_source;
        }
        else {
            die("Invalid C source '$c_source'");
        }
    }
    for my $c_file (@$c_files) {
        my $o_file   = $c_file;
        my $ccs_file = $c_file;
        $o_file   =~ s/\.c$/$Config{_o}/ or die "no match";
        $ccs_file =~ s/\.c$/.ccs/        or die "no match";
        push @objects, $o_file;
        next if $self->up_to_date( $c_file, $o_file );
        $self->add_to_cleanup($o_file);
        $self->add_to_cleanup($ccs_file);
        $cbuilder->compile(
            source               => $c_file,
            extra_compiler_flags => $self->extra_compiler_flags,
            include_dirs         => $self->include_dirs,
            object_file          => $o_file,
        );
    }

    # .xs => .c
    my $xs_filepath         = catfile( $libdir, "$class_name.xs" );
    my $perl_binding_c_file = catfile( $libdir, "$class_name.c" );
    $self->add_to_cleanup($perl_binding_c_file);
    if ( !$self->up_to_date( $xs_filepath, $perl_binding_c_file ) ) {
        ExtUtils::ParseXS::process_file(
            filename   => $xs_filepath,
            prototypes => 0,
            output     => $perl_binding_c_file,
        );
    }

    # .c => .o
    my $version = $self->dist_version;
    my $perl_binding_o_file = catfile( $libdir, "$class_name$Config{_o}" );
    unshift @objects, $perl_binding_o_file;
    $self->add_to_cleanup($perl_binding_o_file);
    if ( !$self->up_to_date( $perl_binding_c_file, $perl_binding_o_file ) ) {
        $cbuilder->compile(
            source               => $perl_binding_c_file,
            extra_compiler_flags => $self->extra_compiler_flags,
            include_dirs         => $self->include_dirs,
            object_file          => $perl_binding_o_file,
            # 'defines' is an undocumented parameter to compile(), so we
            # should officially roll our own variant and generate compiler
            # flags.  However, that involves writing a bunch of
            # platform-dependent code, so we'll just take the chance that this
            # will break.
            defines => {
                VERSION    => qq|"$version"|,
                XS_VERSION => qq|"$version"|,
            },
        );
    }

    # Create .bs bootstrap file, needed by Dynaloader.
    my $bs_file = catfile( $archdir, "$class_name.bs" );
    $self->add_to_cleanup($bs_file);
    if ( !$self->up_to_date( $perl_binding_o_file, $bs_file ) ) {
        require ExtUtils::Mkbootstrap;
        ExtUtils::Mkbootstrap::Mkbootstrap($bs_file);
        if ( !-f $bs_file ) {
            # Create file in case Mkbootstrap didn't do anything.
            open( my $fh, '>', $bs_file )
                or confess "Can't open $bs_file: $!";
        }
        utime( (time) x 2, $bs_file );    # touch
    }

    # Clean up after CBuilder under MSVC.
    $self->add_to_cleanup('compilet*');
    $self->add_to_cleanup('*.ccs');
    $self->add_to_cleanup( catfile( $libdir, "$class_name.ccs" ) );
    $self->add_to_cleanup( catfile( $libdir, "$class_name.def" ) );
    $self->add_to_cleanup( catfile( $libdir, "${class_name}_def.old" ) );
    $self->add_to_cleanup( catfile( $libdir, "$class_name.exp" ) );
    $self->add_to_cleanup( catfile( $libdir, "$class_name.lib" ) );
    $self->add_to_cleanup( catfile( $libdir, "$class_name.lds" ) );
    $self->add_to_cleanup( catfile( $libdir, "$class_name.base" ) );

    # .o => .(a|bundle)
    my $lib_file = catfile( $archdir, "$class_name.$Config{dlext}" );
    if ( !$self->up_to_date( [ @objects, $AUTOGEN_DIR ], $lib_file ) ) {
        $cbuilder->link(
            module_name        => $module_name,
            objects            => \@objects,
            lib_file           => $lib_file,
            extra_linker_flags => $self->extra_linker_flags,
        );
    }
}

sub ACTION_code {
    my $self = shift;

    $self->dispatch('clownfish');
    $self->dispatch('compile_custom_xs');
    $self->dispatch('copy_clownfish_includes');

    $self->SUPER::ACTION_code;
}

1;

__END__