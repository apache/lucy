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
use File::Spec::Functions qw( catfile catdir no_upwards );
use File::Copy qw( copy );
use Cwd qw( getcwd );

if ( @ARGV != 2 ) {
    die "Usage: perl update_snowstem.pl SNOWBALL_SVN_CO LUCY_SNOWSTEM_DIR";
}

my ( $snow_co_dir, $dest_dir ) = @ARGV;
die("Not a directory: '$snow_co_dir'") unless -d $snow_co_dir;

my $retval = system( "svn", "update", "-r", "541", $snow_co_dir );
die "svn update failed" if ( $retval >> 8 );

my $oldpwd = getcwd();
my $snow_build_dir = catdir( $snow_co_dir, 'snowball' );
chdir($snow_build_dir) or die $!;
$retval = system("make dist_libstemmer_c");
die "'make dist_libstemmer_c' failed" if ( $retval >> 8 );
chdir($oldpwd) or die $!;

# Copy only UTF-8 Stemmer files.  Keep directory structure intact so that
# compilation succeeds.
copy_dir_contents( 'src_c', qr/UTF/ );
copy_dir_contents('include');
copy_dir_contents('runtime');
copy_dir_contents( 'libstemmer', qr/utf8.[ch]$/ );

# Add include guard to libstemmer.h.
my $libstemmer_h_path
    = catfile( $dest_dir, qw( source include libstemmer.h ) );
open( my $libstemmer_h_fh, '<', $libstemmer_h_path )
    or die "Can't open '$libstemmer_h_path': $!";
my $libstemmer_h_content = do { local $/; <$libstemmer_h_fh> };
close $libstemmer_h_fh or die $!;
open( $libstemmer_h_fh, '>', $libstemmer_h_path )
    or die "Can't open '$libstemmer_h_path': $!";
print $libstemmer_h_fh <<END_STUFF;
#ifndef H_LIBSTEMMER
#define H_LIBSTEMMER

$libstemmer_h_content

#endif /* H_LIBSTEMMER */

END_STUFF

sub copy_dir_contents {
    my ( $dir_name, $pattern ) = @_;
    my $from_dir = catdir( $snow_build_dir, $dir_name );
    my $to_dir = catdir( $dest_dir, 'source', $dir_name );
    opendir( my $dh, $from_dir )
        or die "Can't opendir '$from_dir': $!";
    die "Not a directory: '$to_dir'" unless -d $to_dir;
    for my $file ( no_upwards( readdir $dh ) ) {
        next if $pattern && $file !~ $pattern;
        next if $file =~ /\.svn/;
        my $from = catfile( $from_dir, $file );
        my $to   = catfile( $to_dir,   $file );
        copy( $from, $to ) or die "Can't copy '$from' to '$to': $!";
    }
    closedir $dh or die $!;
}

