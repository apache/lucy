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

package Lucy::Build;
use base qw(
    Clownfish::CFC::Perl::Build
    Clownfish::CFC::Perl::Build::Charmonic
);

our $VERSION = '0.006002';
$VERSION = eval $VERSION;

use File::Spec::Functions qw( catdir catfile rel2abs updir );
use File::Path qw( rmtree );
use File::Copy qw( move );
use Config;
use Carp;
use Cwd qw( getcwd );

my $LIB_DIR      = 'lib';
my $IS_CPAN_DIST = -e 'cfcore';
my $CHARMONIZER_C;
if ($IS_CPAN_DIST) {
    $CHARMONIZER_C = 'charmonizer.c';
}
else {
    $CHARMONIZER_C = catfile( updir(), 'common', 'charmonizer.c' );
}

sub new {
    my $self = shift->SUPER::new( recursive_test_files => 1, @_ );

    # Fix for MSVC: Although the generated XS should be C89-compliant, it
    # must be compiled in C++ mode like the rest of the code due to a
    # mismatch between the sizes of the C++ bool type and the emulated bool
    # type. (The XS code is compiled with Module::Build's extra compiler
    # flags, not the Clownfish cflags.)
    if ($Config{cc} =~ /^cl\b/) {
        my $extra_cflags = $self->extra_compiler_flags;
        push @$extra_cflags, '/TP';
        $self->extra_compiler_flags(@$extra_cflags);
    }

    if ( $ENV{LUCY_VALGRIND} ) {
        my $optimize = $self->config('optimize') || '';
        $optimize =~ s/\-O\d+/-O1/g;
        $self->config( optimize => $optimize );
    }

    $self->charmonizer_params( create_makefile => 1 );
    $self->charmonizer_params( charmonizer_c => $CHARMONIZER_C );

    $self->clownfish_params( autogen_header => $self->autogen_header );

    return $self;
}

sub ACTION_copy_clownfish_includes {
    my $self = shift;

    $self->SUPER::ACTION_copy_clownfish_includes;

    $self->cf_copy_include_file( qw( Lucy Util ToolSet.h ) );
}

sub _valgrind_base_command {
    return
          "PERL_DESTRUCT_LEVEL=2 LUCY_VALGRIND=1 valgrind "
        . "--leak-check=yes "
        . "--show-reachable=yes "
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
    # Debian's debugperl uses the Config.pm of the standard system perl
    # so -DDEBUGGING won't be detected.
    die "Must be run under a perl that was compiled with -DDEBUGGING"
        unless $self->config('ccflags') =~ /-D?DEBUGGING\b/
               || $^X =~ /\bdebugperl\b/;
    if ( !$ENV{LUCY_VALGRIND} ) {
        warn "\$ENV{LUCY_VALGRIND} not true -- possible false positives";
    }
    $self->depends_on('code');

    # Unbuffer STDOUT, grab test file names.
    $|++;
    my $t_files = $self->find_test_files;    # not public M::B API, may fail
    my $valgrind_command = $self->_valgrind_base_command;

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

sub ACTION_clownfish {
    my $self = shift;

    $self->SUPER::ACTION_clownfish;

    # Make sure to remove empty directory.
    $self->add_to_cleanup( catdir( $LIB_DIR, 'Lucy', 'Docs' ) );
}

sub ACTION_compile_custom_xs {
    my $self = shift;

    $self->depends_on(qw( charmony ));

    # Add extra compiler flags from Charmonizer.
    my $charm_cflags = $self->charmony('EXTRA_CFLAGS');
    if ($charm_cflags) {
        my $cf_cflags = $self->clownfish_params('cflags');
        if ($cf_cflags) {
            $cf_cflags .= " $charm_cflags";
        }
        else {
            $cf_cflags = $charm_cflags;
        }
        $self->clownfish_params( cflags => $cf_cflags );
    }

    $self->SUPER::ACTION_compile_custom_xs;
}

sub autogen_header {
    my $self = shift;
    return <<"END_AUTOGEN";
***********************************************

!!!! DO NOT EDIT !!!!

This file was auto-generated by Build.PL.

***********************************************

Licensed to the Apache Software Foundation (ASF) under one or more
contributor license agreements.  See the NOTICE file distributed with
this work for additional information regarding copyright ownership.
The ASF licenses this file to You under the Apache License, Version 2.0
(the "License"); you may not use this file except in compliance with
the License.  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
END_AUTOGEN
}

sub ACTION_dist {
    my $self = shift;

    die("Module::Build 0.40_11 is required for ./Build dist")
        if $Module::Build::VERSION < 0.40_11;

    # Create POD.
    $self->depends_on('pod');
    rmtree("autogen");

    # We build our Perl release tarball from $REPOS_ROOT/perl, rather than
    # from the top-level.
    #
    # Because some items we need are outside this directory, we need to copy a
    # bunch of stuff.  After the tarball is packaged up, we delete the copied
    # directories.
    my %to_copy = (
        '../core'                          => 'cfcore',
        '../test'                          => 'cftest',
        '../modules'                       => 'modules',
        '../devel'                         => 'devel',
        '../lemon'                         => 'lemon',
        '../common/sample/us_constitution' => 'sample/us_constitution',
        '../CHANGES'                       => 'CHANGES',
        '../CONTRIBUTING'                  => 'CONTRIBUTING',
        '../LICENSE'                       => 'LICENSE',
        '../NOTICE'                        => 'NOTICE',
        '../README'                        => 'README',
        $CHARMONIZER_C                     => 'charmonizer.c',
    );
    print "Copying files...\n";
    while (my ($from, $to) = each %to_copy) {
        confess("'$to' already exists") if -e $to;
        system("cp -R $from $to") and confess();
    }

    move( "MANIFEST", "MANIFEST.bak" ) or die "move() failed: $!";
    $self->depends_on('manifest');
    $self->SUPER::ACTION_dist;

    # Clean up.
    print "Removing copied files...\n";
    rmtree($_) for values %to_copy;
    unlink("META.yml");
    unlink("META.json");
    move( "MANIFEST.bak", "MANIFEST" ) or die "move() failed: $!";
}

sub ACTION_semiclean {
    my $self = shift;
    print "Cleaning up most build files.\n";
    my @candidates
        = grep { $_ !~ /(charmonizer|^_charm|charmony|charmonize|snowstem)/ }
        $self->cleanup;
    for my $path ( map { glob($_) } @candidates ) {
        next unless -e $path;
        rmtree($path);
        confess("Failed to remove '$path'") if -e $path;
    }
}

1;

__END__
