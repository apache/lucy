use strict;
use warnings;

package Lucy::Build::CBuilder;
BEGIN { our @ISA = "ExtUtils::CBuilder"; }
use Config;

sub new {
    my $class = shift;
    require ExtUtils::CBuilder;
    return $class->SUPER::new(@_);
}

# This method isn't implemented by CBuilder for Windows, so we issue a basic
# link command that works on at least one system and hope for the best.
sub link_executable {
    my ( $self, %args ) = @_;
    if ( $Config{cc} eq 'cl' ) {
        my ( $objects, $exe_file ) = @args{qw( objects exe_file )};
        $self->do_system("link /out:$exe_file @$objects");
        return $exe_file;
    }
    else {
        return $self->SUPER::link_executable(%args);
    }
}

package Lucy::Build;
use base qw( Module::Build );

use File::Spec::Functions
    qw( catdir catfile curdir splitpath updir no_upwards );
use File::Path qw( mkpath rmtree );
use File::Copy qw( copy move );
use File::Find qw( find );
use Config;
use Env qw( @PATH );

unshift @PATH, curdir();

=for Rationale

When the distribution tarball for the Perl binding of Lucy is built, c_src/,
c_test/, and any other needed files/directories are copied into the perl/
directory within the main Lucy directory.  Then the distro is built from the
contents of the perl/ directory, leaving out all the files in ruby/, etc.
However, during development, the files are accessed from their original
locations.

=cut

my $is_distro_not_devel = -e 'c_src';
my $base_dir = $is_distro_not_devel ? curdir() : updir();

my $CHARMONIZE_EXE_PATH  = 'charmonize' . $Config{_exe};
my $CHARMONIZER_ORIG_DIR = catdir( $base_dir, 'charmonizer' );
my $CHARMONIZER_GEN_DIR  = catdir( $CHARMONIZER_ORIG_DIR, 'gen' );
my $C_SOURCE_DIR         = catdir( $base_dir, 'c_src' );
my $H_SOURCE_DIR         = catdir( $C_SOURCE_DIR, 'h' );
my $XS_SOURCE_DIR        = 'xs';
my $AUTOBIND_PM_PATH     = catfile(qw( lib KinoSearch Autobinding.pm ));
my $AUTOBIND_XS_PATH     = catfile(qw( lib KinoSearch Autobinding.xs ));

my $EXTRA_CCFLAGS = '';
if ( $ENV{LUCY_DEBUG} ) {
    $EXTRA_CCFLAGS = "-DPERL_GCC_PEDANTIC -ansi -pedantic -Wall -Wextra "
        . "-std=c89 -Wno-long-long -Wno-variadic-macros";
}
my $VALGRIND = $ENV{CHARM_VALGRIND} ? "valgrind --leak-check=full " : "";

# Collect all relevant Charmonizer files.
sub ACTION_metaquote {
    my $self          = shift;
    my $charm_src_dir = catdir( $CHARMONIZER_ORIG_DIR, 'src' );
    my $orig_files    = $self->rscan_dir( $charm_src_dir, qr/\.c?harm$/ );
    my $dest_files    = $self->rscan_dir( $CHARMONIZER_GEN_DIR, qr/\.[ch]$/ );
    push @$dest_files, $CHARMONIZER_GEN_DIR;
    if ( !$self->up_to_date( $orig_files, $dest_files ) ) {
        mkpath $CHARMONIZER_GEN_DIR unless -d $CHARMONIZER_GEN_DIR;
        $self->add_to_cleanup($CHARMONIZER_GEN_DIR);
        my $metaquote = catfile( $CHARMONIZER_ORIG_DIR, qw( bin metaquote ) );
        my $command = "$^X $metaquote --src=$charm_src_dir "
            . "--out=$CHARMONIZER_GEN_DIR";
        system($command);
    }
}

# Build the charmonize executable.
sub ACTION_charmonizer {
    my $self = shift;
    $self->dispatch('metaquote');

    # Gather .c and .h Charmonizer files.
    my $charm_source_files
        = $self->rscan_dir( $CHARMONIZER_GEN_DIR, qr/Charmonizer.+\.[ch]$/ );
    my $charmonize_c = catfile( $CHARMONIZER_ORIG_DIR, 'charmonize.c' );
    my @all_source = ( $charmonize_c, @$charm_source_files );

    # don't compile if we're up to date
    return if $self->up_to_date( \@all_source, $CHARMONIZE_EXE_PATH );

    print "Building $CHARMONIZE_EXE_PATH...\n\n";

    my $cbuilder = Lucy::Build::CBuilder->new;

    my @o_files;
    for (@all_source) {
        next unless /\.c$/;
        next if m#Charmonizer/Test#;
        my $o_file = $cbuilder->object_file($_);
        $self->add_to_cleanup($o_file);
        push @o_files, $o_file;

        next if $self->up_to_date( $_, $o_file );

        $cbuilder->compile(
            source               => $_,
            include_dirs         => [$CHARMONIZER_GEN_DIR],
            extra_compiler_flags => $EXTRA_CCFLAGS,
        );
    }

    $self->add_to_cleanup($CHARMONIZE_EXE_PATH);
    my $exe_path = $cbuilder->link_executable(
        objects  => \@o_files,
        exe_file => $CHARMONIZE_EXE_PATH,
    );
}

# Run the charmonizer executable, creating the charmony.h file.
sub ACTION_charmony {
    my $self          = shift;
    my $charmony_in   = 'charmony_in';
    my $charmony_path = 'charmony.h';

    $self->dispatch('charmonizer');

    return if $self->up_to_date( $CHARMONIZE_EXE_PATH, $charmony_path );
    print "\nWriting $charmony_path...\n\n";

    # write the infile with which to communicate args to charmonize
    my $os_name   = lc( $Config{osname} );
    my $flags     = "$Config{ccflags} $EXTRA_CCFLAGS";
    my $verbosity = $ENV{DEBUG_CHARM} ? 2 : 1;
    my $cc        = "$Config{cc}";
    open( my $infile_fh, '>', $charmony_in )
        or die "Can't open '$charmony_in': $!";
    print $infile_fh qq|
        <charm_os_name>$os_name</charm_os_name>
        <charm_cc_command>$cc</charm_cc_command>
        <charm_cc_flags>$flags</charm_cc_flags>
        <charm_verbosity>$verbosity</charm_verbosity>
    |;
    close $infile_fh or die "Can't close '$charmony_in': $!";

    if ($VALGRIND) {
        system("$VALGRIND ./$CHARMONIZE_EXE_PATH $charmony_in");
    }
    else {
        system( $CHARMONIZE_EXE_PATH, $charmony_in );
    }

    $self->add_to_cleanup( $charmony_path, $charmony_in );
}

sub ACTION_build_charm_test {
    my $self = shift;

    $self->dispatch('charmony');

    # collect source files
    my $source_path = catfile( $base_dir, 'charmonizer', 'charm_test.c' );
    my $exe_path = "charm_test$Config{_exe}";
    my $test_source_dir
        = catdir( $CHARMONIZER_GEN_DIR, qw( Charmonizer Test ) );
    my $source_files = $self->rscan_dir( $CHARMONIZER_GEN_DIR,
        sub { $File::Find::name =~ m#Charmonizer/Test.*?\.c$# } );
    push @$source_files, $source_path;

    # collect include dirs
    my @include_dirs = ( $CHARMONIZER_GEN_DIR, curdir() );

    # add Windows supplements
    if ( $Config{osname} =~ /mswin/i ) {
        my $win_compat_dir = catdir( $base_dir, 'c_src', 'compat' );
        push @include_dirs, $win_compat_dir;
        my $win_compat_files = $self->rscan_dir( $win_compat_dir,
            sub { $File::Find::name =~ m#\.c$# } );
        push @$source_files, @$win_compat_files;
    }

    return if $self->up_to_date( $source_files, $exe_path );

    my $cbuilder = Lucy::Build::CBuilder->new;

    # compile and link "charm_test"
    my @o_files;
    for (@$source_files) {
        my $o_file = $cbuilder->compile(
            source               => $_,
            extra_compiler_flags => $EXTRA_CCFLAGS,
            include_dirs         => \@include_dirs,
        );
        push @o_files, $o_file;
    }
    $cbuilder->link_executable(
        objects  => \@o_files,
        exe_file => $exe_path,
    );

    $self->add_to_cleanup( @o_files, $exe_path );
}

sub ACTION_code {
    my $self = shift;
    $self->dispatch('build_charm_test');
    $self->SUPER::ACTION_code(@_);
}

# copied from Module::Build::Base.pm, added exclude '#' and follow symlinks
sub rscan_dir {
    my ( $self, $dir, $pattern ) = @_;
    my @result;
    local $_;    # find() can overwrite $_, so protect ourselves
    my $subr
        = !$pattern ? sub { push @result, $File::Find::name }
        : !ref($pattern)
        || ( ref $pattern eq 'Regexp' )
        ? sub { push @result, $File::Find::name if /$pattern/ }
        : ref($pattern) eq 'CODE'
        ? sub { push @result, $File::Find::name if $pattern->() }
        : die "Unknown pattern type";

    File::Find::find( { wanted => $subr, no_chdir => 1, follow => 1 }, $dir );

    # skip emacs lock files
    my @filtered = grep !/#/, @result;
    return \@filtered;
}

1;

__END__


    /**
     * Copyright 2006 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

