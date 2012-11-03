#!/usr/bin/perl

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

use Getopt::Long;
use File::Spec::Functions qw( rel2abs catdir catfile updir );
use FindBin qw( $Bin );
use File::stat qw( stat );

# Process command line arguments.
my ( @probes, @user_files, @charm_files );
my $outfile;
GetOptions(
    'probes:s' => \@probes,
    'files=s'  => \@user_files,
    'out=s'    => \$outfile,
);
my $usage = <<END_USAGE;
Usage: 

    meld.pl --files=FILES [--probes=PROBES] --out=OUTFILE

    * probes -- A comma separated list of Charmonizer Probe names, e.g.
      "Integers", "Integers,Floats,LargeFiles".  Defaults to all probes if not
      supplied.
    * files -- A comma separated list of files to be appended verbatim.  At
      least one file must be present and it must define main().
    * out -- The output file.

END_USAGE
die $usage unless @user_files;
die $usage unless $outfile;
$outfile    = rel2abs($outfile);
@user_files = split( /,/, join( ',', @user_files ) );
@probes     = split( /,/, join( ',', @probes ) );
@user_files = map { rel2abs($_) } @user_files;

# Make sure we are in the charmonizer dir.
chdir( catdir( $Bin, updir() ) );

# Default to including all Probes.
if ( !@probes ) {
    my $probe_dir = catdir(qw( src Charmonizer Probe ));
    opendir( my $dh, $probe_dir ) or die "Can't opendir '$probe_dir': $!";
    @probes = map { $_ =~ s/\.c$//; $_ } grep {/\.c$/} readdir $dh;
}

my @core = qw(
    Compiler
    ConfWriter
    ConfWriterC
    ConfWriterPerl
    ConfWriterRuby
    HeaderChecker
    OperatingSystem
    Util
);

# Add Core headers.
for ( 'Defines', @core ) {
    push @charm_files, catfile( qw( src Charmonizer Core ), "$_.h" );
}
push @charm_files, catfile(qw( src Charmonizer Probe.h ));

# Add specified Probe headers in lexically sorted order.
for ( sort @probes ) {
    push @charm_files, catfile( qw( src Charmonizer Probe ), "$_.h" );
}

# Add Core implementation files.
for (@core) {
    push @charm_files, catfile( qw( src Charmonizer Core ), "$_.c" );
}
push @charm_files, catfile(qw( src Charmonizer Probe.c ));

# Add Probe implementation files in lexically sorted order.
for ( sort @probes ) {
    push @charm_files, catfile( qw( src Charmonizer Probe ), "$_.c" );
}

# Don't write out unless there has been an update.
if ( -e $outfile ) {
    my $outfile_mtime = stat($outfile)->mtime;
    my $needs_update;
    for my $dependency ( @charm_files, @user_files ) {
        die "Can't find '$dependency'" unless -e $dependency;
        if ( stat($dependency)->mtime > $outfile_mtime ) {
            $needs_update = 1;
            last;
        }
    }
    if ($needs_update) {
        unlink $outfile;
    }
    else {
        exit;
    }
}

# Start the composite.
open( my $out_fh, '>', $outfile )
    or die "Can't open '$outfile': $!";
print $out_fh meld_start();

# Process core files.
for my $file (@charm_files) {
    print $out_fh pare_charm_file($file);
}

# Process user specified files.
for my $file (@user_files) {
    my $content = slurp($file);

    # Comment out pound-includes for files being inlined.
    $content =~ s|^(#include "Charmonizer[^\n]+)\n|/* $1 */\n|msg;

    print $out_fh qq|#line 1 "$file"\n|;
    print $out_fh $content;
}

close $out_fh or die "Can't close '$outfile': $!";
exit;

sub pare_charm_file {
    my $path    = shift;
    my $content = slurp($path);
    my $num_newlines = $content =~ tr/\n/\n/;

    # Strip license header.
    $content =~ s#/\* Licensed to the Apache.+?\*/\n+##s
        or die "Couldn't find ASF license header in '$path'";

    # Remove opening C++ guards (if this is a header).
    $content =~ s/^#ifdef __cplusplus.*?#endif\n+//ms;

    # Add a #line directive.
    my $new_num_newlines = $content =~ tr/\n/\n/;
    my $starting_line = 1 + $num_newlines - $new_num_newlines;
    $content = qq|#line $starting_line "$path"\n$content|;

    # Remove closing C++ guards (if this is a header).
    $content =~ s/^#ifdef __cplusplus.*?#endif\n+//ms;

    # Comment out pound-includes for files being inlined.
    $content =~ s|^(#include "Charmonizer[^\n]+)\n|/* $1 */\n|msg;

    return <<END_STUFF;
/***************************************************************************/

$content
END_STUFF
}

sub slurp {
    my $path = shift;
    open( my $fh, '<', $path ) or die "Can't open '$path': $!";
    return do { local $/; <$fh> };
}

sub meld_start {
    return <<END_STUFF;
/* This is an auto-generated file -- do not edit directly. */

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

END_STUFF
}

