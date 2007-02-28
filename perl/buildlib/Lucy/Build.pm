use strict;
use warnings;

package Lucy::Build;
use base qw( Module::Build );

# Don't crash Build.PL if CBuilder isn't installed yet
BEGIN { eval "use ExtUtils::CBuilder;"; }

use File::Spec::Functions
    qw( catdir catfile curdir splitpath updir no_upwards );
use File::Path qw( mkpath rmtree );
use File::Copy qw( copy move );
use File::Find qw( find );
use Config;
use Env qw( @PATH );

unshift @PATH, curdir();

my $is_distro_not_devel = -e 'charmonizer';
my $base_dir            = $is_distro_not_devel ? curdir() : updir();

my $METAQUOTE_EXE_PATH     = 'metaquote' . $Config{_exe};
my $CHARMONIZE_EXE_PATH    = 'charmonize' . $Config{_exe};
my $CHARMONIZER_SOURCE_DIR = catdir( $base_dir, 'charmonizer', 'src' );
my $FILTERED_DIR = catdir( $base_dir, qw( charmonizer filtered_src ) );
my $C_SOURCE_DIR = catdir( $base_dir, 'c_src' );
my $R_SOURCE_DIR = catdir( $C_SOURCE_DIR, 'r' );

my $EXTRA_CCFLAGS = '';
if ( $ENV{LUCY_DEBUG} ) {
    $EXTRA_CCFLAGS = "-DPERL_GCC_PEDANTIC -ansi -pedantic -Wall -Wextra "
        . "-std=c89 -Wno-long-long -Wno-variadic-macros";
}
my $VALGRIND = $ENV{CHARM_VALGRIND} ? "valgrind --leak-check=full " : "";

# Compile the metaquote source filter utility.
sub ACTION_metaquote {
    my $self = shift;
    my $source_path
        = catfile( $base_dir, qw( charmonizer metaquote_src metaquote.c ) );

    # don't compile if we're up to date
    return if $self->up_to_date( [$source_path], $METAQUOTE_EXE_PATH );

    # compile
    print "\nBuilding $METAQUOTE_EXE_PATH...\n\n";
    my $cbuilder = ExtUtils::CBuilder->new;
    my $o_file   = $cbuilder->compile(
        source               => $source_path,
        extra_compiler_flags => $EXTRA_CCFLAGS,
    );
    $cbuilder->link_executable(
        objects  => [$o_file],
        exe_file => $METAQUOTE_EXE_PATH,
    );

    # clean both the object file and the executable
    $self->add_to_cleanup( $o_file, $METAQUOTE_EXE_PATH );
}

# Build the charmonize executable.
sub ACTION_charmonizer {
    my $self = shift;

    $self->dispatch('metaquote');

    # gather .charm and .harm files and run them through metaquote
    if ( !-d $FILTERED_DIR ) {
        mkpath($FILTERED_DIR) or die "can't mkpath '$FILTERED_DIR': $!";
    }
    my $charm_source_files = $self->_find_files( $CHARMONIZER_SOURCE_DIR,
        sub { $File::Find::name =~ /\.c?harm$/ } );
    my $filtered_files = $self->_metaquote_charm_files($charm_source_files);
    my $charmonize_c   = catfile( $base_dir, qw( charmonizer charmonize.c ) );
    my @all_source     = ( $charmonize_c, @$filtered_files );

    # don't compile if we're up to date
    return if $self->up_to_date( \@all_source, $CHARMONIZE_EXE_PATH );

    print "Building $CHARMONIZE_EXE_PATH...\n\n";

    my $cbuilder = ExtUtils::CBuilder->new;

    my @o_files;
    for (@all_source) {
        next unless /\.c$/;
        next if m#Charmonizer/Test#;
        my $o_file = $cbuilder->object_file($_);
        push @o_files, $o_file;

        next if $self->up_to_date( $_, $o_file );

        $cbuilder->compile(
            source               => $_,
            include_dirs         => [$FILTERED_DIR],
            extra_compiler_flags => $EXTRA_CCFLAGS,
        );
    }

    my $exe_path = $cbuilder->link_executable(
        objects  => \@o_files,
        exe_file => $CHARMONIZE_EXE_PATH,
    );

    $self->add_to_cleanup( $FILTERED_DIR, @$filtered_files, @o_files,
        $CHARMONIZE_EXE_PATH, );
}

sub _find_files {
    my ( $self, $dir, $test_sub ) = @_;
    my @files;
    find(
        {   wanted => sub {
                if ( $test_sub->() and $File::Find::name !~ /\.\.?$/ ) {
                    push @files, $File::Find::name;
                }
            },
            no_chdir => 1,
        },
        $dir,
    );
    return \@files;
}

sub _metaquote_charm_files {
    my ( $self, $charm_files ) = @_;
    my @filtered_files;

    for my $source_path (@$charm_files) {
        my $dest_path = $source_path;
        $dest_path =~ s#(.*)src#$1filtered_src#;
        $dest_path =~ s#\.charm#.c#;
        $dest_path =~ s#\.harm#.h#;

        push @filtered_files, $dest_path;

        next if ( $self->up_to_date( $source_path, $dest_path ) );

        # create directories if need be
        my ( undef, $dir, undef ) = splitpath($dest_path);
        if ( !-d $dir ) {
            $self->add_to_cleanup($dir);
            mkpath $dir or die "Couldn't mkpath $dir";
        }

        # run the metaquote filter
        system( $METAQUOTE_EXE_PATH, $source_path, $dest_path );
    }

    return \@filtered_files;
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
    my $source_path     = catfile( $base_dir, 'charmonizer', 'charm_test.c' );
    my $exe_path        = "charm_test$Config{_exe}";
    my $test_source_dir = catdir( $FILTERED_DIR, qw( Charmonizer Test ) );
    my $source_files    = $self->_find_files( $FILTERED_DIR,
        sub { $File::Find::name =~ m#Charmonizer/Test.*?\.c$# } );
    push @$source_files, $source_path;

    # collect include dirs
    my @include_dirs = ( $FILTERED_DIR, curdir() );

    # add Windows supplements
    if ( $Config{osname} =~ /mswin/i ) {
        my $win_compat_dir = catdir( $base_dir, 'c_src', 'compat' );
        push @include_dirs, $win_compat_dir;
        my $win_compat_files = $self->_find_files( $win_compat_dir,
            sub { $File::Find::name =~ m#\.c$# } );
        push @$source_files, @$win_compat_files;
    }

    return if $self->up_to_date( $source_files, $exe_path );

    my $cbuilder = ExtUtils::CBuilder->new;

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

