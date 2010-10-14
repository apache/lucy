use strict;
use warnings;

package Clownfish::Binding::Perl;

use Clownfish::Hierarchy;
use Carp;
use File::Spec::Functions qw( catfile );
use Fcntl;

use Clownfish::Parcel;
use Clownfish::Class;
use Clownfish::Function;
use Clownfish::Method;
use Clownfish::Variable;
use Clownfish::Util qw( verify_args a_isa_b write_if_changed );
use Clownfish::Binding::Perl::Class;
use Clownfish::Binding::Perl::Method;
use Clownfish::Binding::Perl::Constructor;

our %new_PARAMS = (
    parcel     => undef,
    hierarchy  => undef,
    lib_dir    => undef,
    boot_class => undef,
    header     => undef,
    footer     => undef,
);

sub new {
    my $either = shift;
    verify_args( \%new_PARAMS, @_ ) or confess $@;
    my $self = bless { %new_PARAMS, @_, }, ref($either) || $either;
    if ( !a_isa_b( $self->{parcel}, 'Clownfish::Parcel' ) ) {
        $self->{parcel}
            = Clownfish::Parcel->singleton( name => $self->{parcel} );
    }
    my $parcel = $self->{parcel};
    for ( keys %new_PARAMS ) {
        confess("$_ is mandatory") unless defined $self->{$_};
    }

    # Derive filenames.
    my $lib                = $self->{lib_dir};
    my $dest_dir           = $self->{hierarchy}->get_dest;
    my @file_components    = split( '::', $self->{boot_class} );
    my @xs_file_components = @file_components;
    $xs_file_components[-1] .= '.xs';
    $self->{xs_path} = catfile( $lib, @xs_file_components );

    $self->{pm_path} = catfile( $lib, @file_components, 'Autobinding.pm' );
    $self->{boot_h_file} = $parcel->get_prefix . "boot.h";
    $self->{boot_c_file} = $parcel->get_prefix . "boot.c";
    $self->{boot_h_path} = catfile( $dest_dir, $self->{boot_h_file} );
    $self->{boot_c_path} = catfile( $dest_dir, $self->{boot_c_file} );

    # Derive the name of the bootstrap function.
    $self->{boot_func}
        = $parcel->get_prefix . $self->{boot_class} . '_bootstrap';
    $self->{boot_func} =~ s/\W/_/g;

    return $self;
}

sub write_bindings {
    my $self           = shift;
    my @ordered        = $self->{hierarchy}->ordered_classes;
    my $registry       = Clownfish::Binding::Perl::Class->registry;
    my $hand_rolled_xs = "";
    my $generated_xs   = "";
    my $xs             = "";
    my @xsubs;

    # Build up a roster of all requested bindings.
    my %has_constructors;
    my %has_methods;
    my %has_xs_code;
    while ( my ( $class_name, $class_binding ) = each %$registry ) {
        $has_constructors{$class_name} = 1
            if $class_binding->get_bind_constructors;
        $has_methods{$class_name} = 1
            if $class_binding->get_bind_methods;
        $has_xs_code{$class_name} = 1
            if $class_binding->get_xs_code;
    }

    # Pound-includes for generated headers.
    for my $class (@ordered) {
        my $include_h = $class->include_h;
        $generated_xs .= qq|#include "$include_h"\n|;
    }
    $generated_xs .= "\n";

    # Constructors.
    for my $class (@ordered) {
        my $class_name = $class->get_class_name;
        next unless delete $has_constructors{$class_name};
        my $class_binding = $registry->{$class_name};
        my @bound         = $class_binding->constructor_bindings;
        $generated_xs .= $_->xsub_def . "\n" for @bound;
        push @xsubs, @bound;
    }

    # Methods.
    for my $class (@ordered) {
        my $class_name = $class->get_class_name;
        next unless delete $has_methods{$class_name};
        my $class_binding = $registry->{$class_name};
        my @bound         = $class_binding->method_bindings;
        $generated_xs .= $_->xsub_def . "\n" for @bound;
        push @xsubs, @bound;
    }

    # Hand-rolled XS.
    for my $class_name ( keys %has_xs_code ) {
        my $class_binding = $registry->{$class_name};
        $hand_rolled_xs .= $class_binding->get_xs_code . "\n";
    }
    %has_xs_code = ();

    # Verify that all binding specs were processed.
    my @leftover_ctor = keys %has_constructors;
    if (@leftover_ctor) {
        confess(  "Constructor bindings spec'd for non-existant classes: "
                . "'@leftover_ctor'" );
    }
    my @leftover_bound = keys %has_methods;
    if (@leftover_bound) {
        confess(  "Method bindings spec'd for non-existant classes: "
                . "'@leftover_bound'" );
    }
    my @leftover_xs = keys %has_xs_code;
    if (@leftover_xs) {
        confess(  "Hand-rolled XS spec'd for non-existant classes: "
                . "'@leftover_xs'" );
    }

    # Build up code for booting XSUBs at module load time.
    my @xs_init_lines;
    for my $xsub (@xsubs) {
        my $c_name    = $xsub->c_name;
        my $perl_name = $xsub->perl_name;
        push @xs_init_lines, qq|newXS("$perl_name", $c_name, file);|;
    }
    my $xs_init = join( "\n    ", @xs_init_lines );

    # Params hashes for arg checking of XSUBs that take labeled params.
    my @params_hash_defs = grep {defined} map { $_->params_hash_def } @xsubs;
    my $params_hash_defs = join( "\n", @params_hash_defs );

    # Write out if there have been any changes.
    my $xs_file_contents = $self->_xs_file_contents( $generated_xs, $xs_init,
        $hand_rolled_xs );
    my $pm_file_contents = $self->_pm_file_contents($params_hash_defs);
    write_if_changed( $self->{xs_path}, $xs_file_contents );
    write_if_changed( $self->{pm_path}, $pm_file_contents );
}

sub _xs_file_contents {
    my ( $self, $generated_xs, $xs_init, $hand_rolled_xs ) = @_;
    return <<END_STUFF;
#include "xs/XSBind.h"
#include "boil.h"
#include "$self->{boot_h_file}"

#include "KinoSearch/Object/Host.h"
#include "KinoSearch/Util/Memory.h"
#include "KinoSearch/Util/StringHelper.h"

#include "Charmonizer/Test.h"
#include "Charmonizer/Test/AllTests.h"

$generated_xs

MODULE = KinoSearch   PACKAGE = KinoSearch::Autobinding

void
init_autobindings()
PPCODE:
{
    char* file = __FILE__;
    CHY_UNUSED_VAR(cv); 
    CHY_UNUSED_VAR(items); $xs_init
}

$hand_rolled_xs

END_STUFF
}

sub _pm_file_contents {
    my ( $self, $params_hash_defs ) = @_;
    return <<END_STUFF;
# DO NOT EDIT!!!! This is an auto-generated file.

use strict;
use warnings;

package KinoSearch::Autobinding;

init_autobindings();

$params_hash_defs

1;

END_STUFF
}

sub prepare_pod {
    my $self    = shift;
    my $lib_dir = $self->{lib_dir};
    my @ordered = $self->{hierarchy}->ordered_classes;
    my @files_written;
    my %has_pod;
    my %modified;

    my $registry = Clownfish::Binding::Perl::Class->registry;
    $has_pod{ $_->get_class_name } = 1
        for grep { $_->get_make_pod } values %$registry;

    for my $class (@ordered) {
        my $class_name = $class->get_class_name;
        my $class_binding = $registry->{$class_name} or next;
        next unless delete $has_pod{$class_name};
        my $pod = $class_binding->create_pod;
        confess("Failed to generate POD for $class_name") unless $pod;

        # Compare against existing file; rewrite if changed.
        my $pod_file_path
            = catfile( $lib_dir, split( '::', $class_name ) ) . ".pod";

        $class->file_path( $lib_dir, ".pod" );
        my $existing = "";
        if ( -e $pod_file_path ) {
            open( my $pod_fh, "<", $pod_file_path )
                or confess("Can't open '$pod_file_path': $!");
            $existing = do { local $/; <$pod_fh> };
        }
        if ( $pod ne $existing ) {
            $modified{$pod_file_path} = $pod;
        }
    }
    my @leftover = keys %has_pod;
    confess("Couldn't match pod to class for '@leftover'") if @leftover;

    return \%modified;
}

sub write_boot {
    my $self = shift;
    $self->_write_boot_h;
    $self->_write_boot_c;
}

sub _write_boot_h {
    my $self      = shift;
    my $hierarchy = $self->{hierarchy};
    my $filepath  = catfile( $hierarchy->get_dest, $self->{boot_h_file} );
    my $guard     = uc("$self->{boot_class}_BOOT");
    $guard =~ s/\W+/_/g;

    unlink $filepath;
    sysopen( my $fh, $filepath, O_CREAT | O_EXCL | O_WRONLY )
        or confess("Can't open '$filepath': $!");
    print $fh <<END_STUFF;
$self->{header}

#ifndef $guard
#define $guard 1

void
$self->{boot_func}();

#endif /* $guard */

$self->{footer}
END_STUFF
}

sub _write_boot_c {
    my $self           = shift;
    my $hierarchy      = $self->{hierarchy};
    my @ordered        = $hierarchy->ordered_classes;
    my $num_classes    = scalar @ordered;
    my $pound_includes = "";
    my $registrations  = "";
    my $isa_pushes     = "";

    for my $class (@ordered) {
        my $include_h = $class->include_h;
        $pound_includes .= qq|#include "$include_h"\n|;
        next if $class->inert;
        my $prefix  = $class->get_prefix;
        my $PREFIX  = $class->get_PREFIX;
        my $vt_type = $PREFIX . $class->vtable_type;

        # Ignore return value from VTable_add_to_registry, since it's OK if
        # multiple threads contend for adding these permanent VTables and some
        # fail.
        $registrations
            .= qq|    ${prefix}VTable_add_to_registry($PREFIX|
            . $class->vtable_var
            . qq|);\n|;

        my $parent = $class->get_parent;
        next unless $parent;
        my $parent_class = $parent->get_class_name;
        my $class_name   = $class->get_class_name;
        $isa_pushes .= qq|    isa = get_av("$class_name\::ISA", 1);\n|;
        $isa_pushes .= qq|    av_push(isa, newSVpv("$parent_class", 0));\n|;
    }
    my $filepath = catfile( $hierarchy->get_dest, $self->{boot_c_file} );
    unlink $filepath;
    sysopen( my $fh, $filepath, O_CREAT | O_EXCL | O_WRONLY )
        or confess("Can't open '$filepath': $!");
    print $fh <<END_STUFF;
$self->{header}

#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"
#include "$self->{boot_h_file}"
#include "boil.h"
$pound_includes

void
$self->{boot_func}()
{
    AV *isa;
$registrations
$isa_pushes
}

$self->{footer}

END_STUFF
}

sub write_xs_typemap {
    my $self = shift;
    Clownfish::Binding::Perl::TypeMap->write_xs_typemap(
        hierarchy => $self->{hierarchy}, );
}

1;

__END__

__POD__

=head1 NAME

Clownfish::Binding::Perl - Perl bindings for a Clownfish::Hierarchy.

=head1 DESCRIPTION

Clownfish::Binding::Perl presents an interface for auto-generating XS and
Perl code to bind C code for a Clownfish class hierarchy to Perl.

In theory this module could be much more flexible and its API could be more
elegant.  There are many ways which you could walk the parsed parcels,
classes, methods, etc. in a Clownfish::Hierarchy and generate binding code.
However, our needs are very limited, so we are content with a "one size fits
one" solution.

In particular, this module assumes that the XS bindings for all classes in the
hierarchy should be assembled into a single shared object which belongs to the
primary, "boot" class.  There's no reason why it could not write one .xs file
per class, or one per parcel, instead.

The files written by this class are derived from the name of the boot class.
If it is "Crustacean", the following files will be generated.

    # Generated by write_bindings()
    $lib_dir/Crustacean.xs
    $lib_dir/Crustacean/Autobinding.pm

    # Generated by write_boot()
    $hierarchy_dest_dir/crust_boot.h
    $hierarchy_dest_dir/crust_boot.c

=head1 METHODS

=head2 new

    my $perl_binding = Clownfish::Binding::Perl->new(
        boot_class => 'Crustacean',                    # required
        parcel     => 'Crustacean',                    # required
        hierarchy  => $hierarchy,                      # required
        lib_dir    => 'lib',                           # required
        header     => "/* Autogenerated file */\n",    # required
        footer     => $copyfoot,                       # required
    );

=over

=item * B<boot_class> - The name of the main class, which will own the shared
object.

=item * B<parcel> - The L<Clownfish::Parcel> to which the C<boot_class>
belongs.

=item * B<hierarchy> - A Clownfish::Hierarchy.

=item * B<lib_dir> - location of the Perl lib directory to which files will be
written.

=item * B<header> - Text which will be prepended to generated C/XS files --
typically, an "autogenerated file" warning.

=item * B<footer> - Text to be appended to the end of generated C/XS files --
typically copyright information.

=back

=head2 write_bindings

    $perl_binding->write_bindings;

Generate the XS bindings (including "Autobind.pm) for all classes in the
hierarchy.

=head2 prepare_pod 

    my $filepaths_and_pod = $perl_binding->prepare_pod;
    while ( my ( $filepath, $pod ) = each %$filepaths_and_pod ) {
        add_to_cleanup($filepath);
        spew_file( $filepath, $pod );
    }

Auto-generate POD for all classes bindings which were spec'd with C<make_pod>
directives.  See whether a .pod file exists and is up to date.

Return a hash representing POD files that need to be modified; the keys are
filepaths, and the values are the POD file content.

The caller must take responsibility for actually writing out the POD files,
after adding the filepaths to cleanup records and so on.

=head2 write_boot

    $perl_binding->write_boot;

Write out "boot" files to the Hierarchy's C<dest_dir> which contain code for
bootstrapping Clownfish classes.

=head1 COPYRIGHT AND LICENSE

Copyright 2008-2010 Marvin Humphrey

This program is free software; you can redistribute it and/or modify it under
the same terms as Perl itself.

=cut

