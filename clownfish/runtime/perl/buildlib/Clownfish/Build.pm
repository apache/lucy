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

use lib '../../compiler/perl/blib/arch';
use lib '../../compiler/perl/blib/lib';

package Clownfish::Build;

# We want to subclass Clownfish::CFC::Perl::Build, but CFC might not be built
# yet. So we look in 'clownfish/compiler/perl/lib' directly and cleanup @INC
# afterwards.
use lib '../../compiler/perl/lib';
use base qw( Clownfish::CFC::Perl::Build );
no lib '../../compiler/perl/lib';

our $VERSION = '0.003000';
$VERSION = eval $VERSION;

use File::Spec::Functions qw( catdir catfile updir rel2abs );
use File::Path qw( rmtree );
use File::Copy qw( move );
use Config;
use Env qw( @PATH );
use Carp;
use Cwd qw( getcwd );

BEGIN { unshift @PATH, rel2abs( getcwd() ) }

my @BASE_PATH = __PACKAGE__->cf_base_path;

my $CHARMONIZER_ORIG_DIR
    = rel2abs( catdir( @BASE_PATH, updir(), updir(), 'charmonizer' ) );
my $CHARMONIZE_EXE_PATH  = "charmonize$Config{_exe}";
my $CHARMONY_H_PATH      = 'charmony.h';
my $CHARMONY_PM_PATH     = 'Charmony.pm';
my $CORE_SOURCE_DIR = catdir( @BASE_PATH, 'core' );
my $CFC_DIR = catdir( @BASE_PATH, updir(), 'compiler', 'perl' );
my $CFC_BUILD  = catfile( $CFC_DIR, 'Build' );
my $LIB_DIR          = 'lib';

sub new {
    my $self = shift->SUPER::new( recursive_test_files => 1, @_ );

    if ( $ENV{LUCY_VALGRIND} ) {
        my $optimize = $self->config('optimize') || '';
        $optimize =~ s/\-O\d+/-O1/g;
        $self->config( optimize => $optimize );
    }

    my $extra_ccflags = $self->extra_compiler_flags;
    if ( $self->config('gccversion') ) {
        if ( $Config{osname} =~ /openbsd/i && !$Config{usethreads} ) {
            push @$extra_ccflags, '-DLUCY_NOTHREADS';
        }
        if ( defined $ENV{LUCY_VALGRIND} ) {
            push @$extra_ccflags, qw( -DLUCY_VALGRIND -fno-inline-functions );
        }
        elsif ( defined $ENV{LUCY_DEBUG} ) {
            push @$extra_ccflags, qw(
                -DLUCY_DEBUG -pedantic -Wall -Wextra -Wno-variadic-macros
            );
        }
    }
    $self->extra_compiler_flags(@$extra_ccflags);

    $self->clownfish_params( autogen_header => $self->autogen_header );

    return $self;
}

sub _run_make {
    my ( $self, %params ) = @_;
    my @command           = @{ $params{args} };
    my $dir               = $params{dir};
    my $current_directory = getcwd();
    chdir $dir if $dir;
    unshift @command, 'CC=' . $self->config('cc');
    if ( $self->config('cc') =~ /^cl\b/ ) {
        unshift @command, "-f", "Makefile.MSVC";
    }
    elsif ( $^O =~ /mswin/i ) {
        unshift @command, "-f", "Makefile.MinGW";
    }
    unshift @command, "$Config{make}";
    system(@command) and confess("$Config{make} failed");
    chdir $current_directory if $dir;
}

# Build the charmonize executable.
sub ACTION_build_charmonize {
    my $self = shift;
    print "Building $CHARMONIZE_EXE_PATH...\n\n";
    my $meld_c = rel2abs("charmonize.c");
    $self->add_to_cleanup($meld_c);
    $self->add_to_cleanup($CHARMONIZE_EXE_PATH);
    my $charmonize_main = catfile( $CHARMONIZER_ORIG_DIR, 'charmonize.c' );
    $self->_run_make(
        dir  => $CHARMONIZER_ORIG_DIR,
        args => [ "meld", "PERL=$^X", "FILES=$charmonize_main", "OUT=$meld_c" ],
    );
    if ( !$self->up_to_date( $meld_c, $CHARMONIZE_EXE_PATH ) ) {
        my $cc = $Config{cc};
        my $outflag = $cc =~ /cl\b/ ? "/Fe" : "-o ";
        system("$cc $meld_c $outflag$CHARMONIZE_EXE_PATH")
            and die "Failed to compile $CHARMONIZE_EXE_PATH";
    }
}

# Run the charmonize executable, creating the charmony.h and Charmony.pm
# files.
sub ACTION_run_charmonize {
    my $self = shift;
    $self->dispatch('build_charmonize');
    return if $self->up_to_date( $CHARMONIZE_EXE_PATH, [
        $CHARMONY_H_PATH, $CHARMONY_PM_PATH,
    ] );
    print "\nRunning $CHARMONIZE_EXE_PATH...\n\n";

    $self->add_to_cleanup($CHARMONY_H_PATH);
    $self->add_to_cleanup($CHARMONY_PM_PATH);
    # Clean up after charmonize if it doesn't succeed on its own.
    $self->add_to_cleanup("_charm*");

    # Prepare arguments to charmonize.
    my @command = (
        $CHARMONIZE_EXE_PATH,
        '--cc=' . _quotify( $self->config('cc') ),
        '--enable-c',
        '--enable-perl',
        '--',
        $self->config('ccflags'),
        @{ $self->extra_compiler_flags },
    );
    if ( $ENV{CHARM_VALGRIND} ) {
        unshift @command, "valgrind", "--leak-check=yes";
    }
    print join( " ", @command ), $/;

    system(@command) and die "Failed to run $CHARMONIZE_EXE_PATH: $!";
}

sub _quotify {
    my $string = shift;
    $string =~ s/\\/\\\\/g;
    $string =~ s/"/\\"/g;
    return qq|"$string"|;
}

# Build the charmonizer tests.
sub ACTION_charmonizer_tests {
    my $self = shift;
    $self->dispatch('run_charmonize');
    print "Building Charmonizer Tests...\n\n";
    my $flags = join( " ",
        $self->config('ccflags'),
        @{ $self->extra_compiler_flags },
        '-I' . rel2abs( getcwd() ),
    );
    $flags =~ s/"/\\"/g;
    $self->_run_make(
        dir  => $CHARMONIZER_ORIG_DIR,
        args => [ "DEFS=$flags", "tests" ],
    );
}

sub ACTION_cfc {
    my $self    = shift;
    my $old_dir = getcwd();
    chdir($CFC_DIR);
    if ( !-f 'Build' ) {
        print "\nBuilding Clownfish compiler... \n";
        system("$^X Build.PL");
        system("$^X Build code");
        print "\nFinished building Clownfish compiler.\n\n";
    }
    chdir($old_dir);
}

sub ACTION_copy_clownfish_includes {
    my $self = shift;

    $self->dispatch('run_charmonize');

    $self->SUPER::ACTION_copy_clownfish_includes;

    $self->cf_copy_include_file( 'charmony.h' );
    $self->cf_copy_include_file( 'XSBind.h' );
}

sub ACTION_clownfish {
    my $self = shift;

    $self->dispatch('charmonizer_tests');
    $self->dispatch('cfc');

    $self->SUPER::ACTION_clownfish;
}

sub ACTION_suppressions {
    my $self       = shift;
    my $LOCAL_SUPP = 'local.supp';
    return
        if $self->up_to_date( '../devel/bin/valgrind_triggers.pl',
        $LOCAL_SUPP );

    # Generate suppressions.
    print "Writing $LOCAL_SUPP...\n";
    $self->add_to_cleanup($LOCAL_SUPP);
    my $command
        = "yes | "
        . $self->_valgrind_base_command
        . "--gen-suppressions=yes "
        . $self->perl
        . " ../devel/bin/valgrind_triggers.pl 2>&1";
    my $suppressions = `$command`;
    $suppressions =~ s/^==.*?\n//mg;
    $suppressions =~ s/^--.*?\n//mg;
    my $rule_number = 1;

    while ( $suppressions =~ /<insert.a.*?>/ ) {
        $suppressions =~ s/^\s*<insert.a.*?>/{\n  <core_perl_$rule_number>/m;
        $rule_number++;
    }

    # Change e.g. fun:_vgrZU_libcZdsoZa_calloc to fun:calloc
    $suppressions =~ s/fun:\w+_((m|c|re)alloc)/fun:$1/g;

    # Write local suppressions file.
    open( my $supp_fh, '>', $LOCAL_SUPP )
        or confess("Can't open '$LOCAL_SUPP': $!");
    print $supp_fh $suppressions;
}

sub _valgrind_base_command {
    return
          "PERL_DESTRUCT_LEVEL=2 LUCY_VALGRIND=1 valgrind "
        . "--leak-check=yes "
        . "--show-reachable=yes "
        . "--num-callers=10 "
        . "--dsymutil=yes "
        . "--suppressions=../devel/conf/lucyperl.supp ";
}

# Run the entire test suite under Valgrind.
#
# For this to work, Lucy must be compiled with the LUCY_VALGRIND environment
# variable set to a true value, under a debugging Perl.
#
# A custom suppressions file will probably be needed -- use your judgment.
# To pass in one or more local suppressions files, provide a comma separated
# list like so:
#
#   $ ./Build test_valgrind --suppressions=foo.supp,bar.supp
sub ACTION_test_valgrind {
    my $self = shift;
    die "Must be run under a perl that was compiled with -DDEBUGGING"
        unless $self->config('ccflags') =~ /-D?DEBUGGING\b/;
    if ( !$ENV{LUCY_VALGRIND} ) {
        warn "\$ENV{LUCY_VALGRIND} not true -- possible false positives";
    }
    $self->dispatch('code');
    $self->dispatch('suppressions');

    # Unbuffer STDOUT, grab test file names and suppressions files.
    $|++;
    my $t_files = $self->find_test_files;    # not public M::B API, may fail
    my $valgrind_command = $self->_valgrind_base_command;
    $valgrind_command .= "--suppressions=local.supp ";

    if ( my $local_supp = $self->args('suppressions') ) {
        for my $supp ( split( ',', $local_supp ) ) {
            $valgrind_command .= "--suppressions=$supp ";
        }
    }

    # Iterate over test files.
    my @failed;
    for my $t_file (@$t_files) {

        # Run test file under Valgrind.
        print "Testing $t_file...";
        die "Can't find '$t_file'" unless -f $t_file;
        my $command = "$valgrind_command $^X -Mblib $t_file 2>&1";
        my $output = "\n" . ( scalar localtime(time) ) . "\n$command\n";
        $output .= `$command`;

        # Screen-scrape Valgrind output, looking for errors and leaks.
        if (   $?
            or $output =~ /ERROR SUMMARY:\s+[^0\s]/
            or $output =~ /definitely lost:\s+[^0\s]/
            or $output =~ /possibly lost:\s+[^0\s]/
            or $output =~ /still reachable:\s+[^0\s]/ )
        {
            print " failed.\n";
            push @failed, $t_file;
            print "$output\n";
        }
        else {
            print " succeeded.\n";
        }
    }

    # If there are failed tests, print a summary list.
    if (@failed) {
        print "\nFailed "
            . scalar @failed . "/"
            . scalar @$t_files
            . " test files:\n    "
            . join( "\n    ", @failed ) . "\n";
        exit(1);
    }
}

sub autogen_header {
    my $self = shift;
    return <<"END_AUTOGEN";
/***********************************************

 !!!! DO NOT EDIT !!!!

 This file was auto-generated by Build.PL.

 ***********************************************/

/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

END_AUTOGEN
}

# Run the cleanup targets for independent prerequisite builds.
sub _clean_prereq_builds {
    my $self = shift;
    if ( -e $CFC_BUILD ) {
        my $old_dir = getcwd();
        chdir $CFC_DIR;
        system("$^X Build realclean")
            and die "Clownfish clean failed";
        chdir $old_dir;
    }
    $self->_run_make( dir => $CHARMONIZER_ORIG_DIR, args => ['clean'] );
}

sub ACTION_clean {
    my $self = shift;
    _clean_prereq_builds($self);
    $self->SUPER::ACTION_clean;
}

1;

