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
use JSON::XS;

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

# Add a README file explaining the deal with Snowball and license headers.
my $snowstem_readme_path = catfile( $dest_dir, 'source', 'README' );
open( my $snowstem_readme_fh, '>:encoding(UTF-8)', $snowstem_readme_path )
    or die "Can't open '$snowstem_readme_path': $!";
print $snowstem_readme_fh <<'END_STUFF';
Unless otherwise noted, the files within this directory are imported directly
from a checkout of the Snowball source code repository.  Some of the files do
not contain license headers, by choice of the Snowball project:

    http://snowball.tartarus.org/license.php

    We have not bothered to insert the licensing arrangement into the text of
    the Snowball software. 

For licensing information, see LICENSE and NOTICE at the top of the Apache
Lucy distribution.
END_STUFF
close $snowstem_readme_fh or die $!;

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

# Write tests.json file.  Only include 10 sample tests for each language to
# save space -- we assume that Snowball is thoroughly exercising its tests
# elsewhere.
my %languages = (
    en => 'english',
    da => 'danish',
    de => 'german',
    es => 'spanish',
    fi => 'finnish',
    fr => 'french',
    it => 'italian',
    nl => 'dutch',
    hu => 'hungarian',
    no => 'norwegian',
    pt => 'portuguese',
    ro => 'romanian',
    ru => 'russian',
    sv => 'swedish',
    tr => 'turkish',
);
my %tests;
for my $iso ( sort keys %languages ) {
    my $language   = $languages{$iso};
    my $words_path = catfile( $snow_co_dir, 'data', $language, 'voc.txt' );
    my $stems_path = catfile( $snow_co_dir, 'data', $language, 'output.txt' );
    open( my $words_fh, '<:encoding(UTF-8)', $words_path )
        or die "Can't open '$words_path': $!";
    open( my $stems_fh, '<:encoding(UTF-8)', $stems_path )
        or die "Can't open '$stems_path': $!";
    my @all_words = <$words_fh>;
    my @all_stems = <$stems_fh>;

    my @some_words;
    my @some_stems;
    my $interval = int( @all_words / 10 );
    for my $i ( 0 .. 9 ) {
        my $word = $all_words[ $i * $interval ];
        my $stem = $all_stems[ $i * $interval ];
        chomp($word);
        chomp($stem);
        die unless length($word) && length($stem);
        push @some_words, $word;
        push @some_stems, $stem;
    }
    $tests{$iso}{words} = \@some_words;
    $tests{$iso}{stems} = \@some_stems;
}
my $json_encoder    = JSON::XS->new->pretty(1)->canonical(1);
my $json            = $json_encoder->encode( \%tests );
my $tests_json_path = catfile( $dest_dir, 'source', 'test', 'tests.json' );
open( my $json_fh, '>:encoding(UTF-8)', $tests_json_path )
    or die "Can't open '$tests_json_path': $!";
print $json_fh $json;
close $json_fh or die $!;

# Write separate README file describing test.json's contents, since JSON is a
# commentless format.
my $readme_path = catfile( $dest_dir, 'source', 'test', 'README' );
open( my $readme_fh, '>:encoding(UTF-8)', $readme_path )
    or die "Can't open '$readme_path': $!";
print $readme_fh <<'END_STUFF';
The file 'tests.json' and this file were autogenerated by update_snowstem.pl.
'tests.json' contains materials from the Snowball project.  See the LICENSE
and NOTICE files for more information.
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

