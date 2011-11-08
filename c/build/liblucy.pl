#!/usr/bin/env perl

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

use File::Spec::Functions
    qw( catdir catfile splitpath updir no_upwards rel2abs );
use File::Path qw( mkpath rmtree );
use File::Copy qw( copy move );
use File::Find qw( find );
use Cwd qw( getcwd );

my $is_distro_not_devel = -e 'core';
my $base_dir = rel2abs( $is_distro_not_devel ? getcwd() : updir() );

my $DEBUG = $ENV{LIBLUCY_DEBUG} || 0;

my $usage = "$0 make-cmd cc-cmd cc-flags\n";

die $usage if $ARGV[0] and $ARGV[0] =~ m/^\-\-?h/;

my $MAKE     = shift(@ARGV) || 'make';
my $CC       = shift(@ARGV) || 'cc';
my $CC_FLAGS = shift(@ARGV) || '';
my $EXE      = shift(@ARGV) || '';

my $CHARMONIZER_ORIG_DIR = catdir( $base_dir, 'charmonizer' );
my $CHARMONIZE_EXE_PATH
    = catfile( $CHARMONIZER_ORIG_DIR, "charmonize${EXE}" );
my $CHARMONY_PATH  = 'charmony.h';
my $LEMON_DIR      = catdir( $base_dir, 'lemon' );
my $LEMON_EXE_PATH = catfile( $LEMON_DIR, "lemon${EXE}" );
my $SNOWSTEM_SRC_DIR
    = catdir( $base_dir, qw( modules analysis snowstem source ) );
my $SNOWSTEM_INC_DIR = catdir( $SNOWSTEM_SRC_DIR, 'include' );
my $SNOWSTOP_SRC_DIR
    = catdir( $base_dir, qw( modules analysis snowstop source ) );
my $CORE_SOURCE_DIR = catdir( $base_dir, 'core' );
my $CLOWNFISH_DIR   = catdir( $base_dir, 'clownfish' );
my $CLOWNFISH_BUILD = catfile( $CLOWNFISH_DIR, 'Build' );

# run this program
main();

sub main {
    build_charmony();
    build_charmonizer_tests();
    build_lemon();

}

sub run_make {
    my (%params)          = @_;
    my @command           = @{ $params{args} };
    my $dir               = $params{dir};
    my $current_directory = getcwd();
    chdir $dir if $dir;
    unshift @command, 'CC=' . $CC;
    if ( $CC =~ /^cl\b/ ) {
        unshift @command, "-f", "Makefile.MSVC";
    }
    elsif ( $^O =~ /mswin/i ) {
        unshift @command, "-f", "Makefile.MinGW";
    }
    unshift @command, $MAKE;
    system(@command) and die("$MAKE failed");
    chdir $current_directory if $dir;
}

sub build_charmonize {
    print "Building $CHARMONIZE_EXE_PATH...\n\n";
    run_make(
        dir  => $CHARMONIZER_ORIG_DIR,
        args => [],
    );
}

# Run the charmonize executable, creating the charmony.h file.
sub build_charmony {
    my $self = shift;
    build_charmonize();

    print "\nWriting $CHARMONY_PATH...\n\n";

    # Clean up after charmonize if it doesn't succeed on its own.
    # TODO
    #$self->add_to_cleanup("_charm*");
    #$self->add_to_cleanup($CHARMONY_PATH);

    # Prepare arguments to charmonize.
    my $flags = $CC_FLAGS;
    $flags =~ s/"/\\"/g;
    my @command = ( $CHARMONIZE_EXE_PATH, $CC, $flags );
    if ( $ENV{CHARM_VALGRIND} ) {
        unshift @command, "valgrind", "--leak-check=yes";
    }

    $DEBUG and print join( " ", @command ), $/;

    system(@command) and die "Failed to write $CHARMONY_PATH: $!";
}

sub build_charmonizer_tests {
    print "Building Charmonizer Tests...\n\n";
    my $flags = join( " ", $CC_FLAGS, '-I' . rel2abs( getcwd() ) );
    $flags =~ s/"/\\"/g;
    run_make(
        dir  => $CHARMONIZER_ORIG_DIR,
        args => [ "DEFS=$flags", "tests" ],
    );
}

sub build_lemon {
    print "Building the Lemon parser generator...\n\n";
    run_make(
        dir  => $LEMON_DIR,
        args => [],
    );
}
