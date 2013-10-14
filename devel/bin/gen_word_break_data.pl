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

=head1 NAME

gen_word_break_data.pl - Generate word break table and tests

=head1 SYNOPSIS

    perl gen_word_break_data.pl [-c] UCD_SRC_DIR

=head1 DESCRIPTION

This script generates the tables to lookup Unicode word break properties
for the StandardTokenizer. It also converts the word break test suite in
the UCD to JSON.

UCD_SRC_DIR should point to a directory containing the files
WordBreakProperty.txt, WordBreakTest.txt, and DerivedCoreProperties.txt from
the Unicode Character Database available at
L<http://www.unicode.org/Public/6.3.0/ucd/>.

=head1 OUTPUT FILES

    modules/unicode/ucd/WordBreak.tab
    modules/unicode/ucd/WordBreakTest.json

=head1 OPTIONS

=head2 -c

Show total table size for different shift values

=cut

use strict;

use Getopt::Std;
use JSON;
use UnicodeTable;

my $output_dir     = '../../modules/unicode/ucd';
my $table_filename = "$output_dir/WordBreak.tab";
my $tests_filename = "$output_dir/WordBreakTest.json";

my %wb_map = (
    CR                 => 0,
    LF                 => 0,
    Newline            => 0,
    Regional_Indicator => 0,  # These are symbols, so ignore them.
    ALetter            => 2,
    Hebrew_Letter      => 3,
    Numeric            => 4,
    Katakana           => 5,
    ExtendNumLet       => 6,
    Extend             => 7,
    Format             => 7,
    Single_Quote       => 8,
    Double_Quote       => 9,
    MidNumLet          => 10,
    MidLetter          => 11,
    MidNum             => 12,
);

my %opts;
if ( !getopts( 'c', \%opts ) || @ARGV != 1 ) {
    print STDERR ("Usage: $0 [-c] UCD_SRC_DIR\n");
    exit;
}

my $src_dir = $ARGV[0];

my $wb = UnicodeTable->read(
    filename => "$src_dir/WordBreakProperty.txt",
    type     => 'Enumerated',
    map      => \%wb_map,
);
my $alpha = UnicodeTable->read(
    filename => "$src_dir/DerivedCoreProperties.txt",
    type     => 'Boolean',
    map      => { Alphabetic => 1 },
);

# Set characters in Alphabetic but not in Word_Break to WB_ASingle = 1
for ( my $i = 0; $i < 0x30000; ++$i ) {
    if ( !$wb->lookup($i) && $alpha->lookup($i) ) {
        $wb->set( $i, 1 );
    }
}

if ( $opts{c} ) {
    $wb->calc_sizes( [ 2, 6 ], [ 3, 9 ] );
    exit;
}

# Optimize for UTF-8
my $row_shift   = 6;
my $plane_shift = 6;

my $wb_ascii = UnicodeTable->new(
    table => [],
    max   => 0,
);

for ( my $i = 0; $i < 0x80; ++$i ) {
    $wb_ascii->set( $i, $wb->lookup($i) );
}

my $wb_rows      = $wb->compress($row_shift);
my $wb_planes    = $wb_rows->compress_map($plane_shift);
my $wb_plane_map = $wb_planes->map_table;

# test compressed table

for ( my $i = 0; $i < 0x110000; ++$i ) {
    my $v1 = $wb->lookup($i);
    my $v2 = $wb_rows->lookup($i);
    die("test for code point $i failed, want $v1, got $v2")
        if $v1 != $v2;
}

# dump tables

open( my $out_file, '>', $table_filename )
    or die("$table_filename: $!\n");

print $out_file (<DATA>);

$wb_ascii->dump( $out_file, 'wb_ascii' );
print $out_file ("\n");
$wb_plane_map->dump( $out_file, 'wb_plane_map' );
print $out_file ("\n");
$wb_planes->dump( $out_file, 'wb_planes' );
print $out_file ("\n");
$wb_rows->dump( $out_file, 'wb_rows' );

close($out_file);

# convert UCD test suite

open( my $in_file, '<', "$src_dir/WordBreakTest.txt" )
    or die("$src_dir/WordBreakTest.txt: $!\n");
binmode( $in_file, ':utf8' );

my @tests;

while (<$in_file>) {
    s/\s*(#.*)?\z//s;
    next if $_ eq '';
    my @items = split(/\s+/);
    my $word  = '';
    my $text  = '';
    my @words;

    for ( my $i = 0; $i + 1 < @items; $i += 2 ) {
        my ( $break, $code ) = ( $items[$i], hex( $items[ $i + 1 ] ) );
        my $chr = chr($code);
        $text .= $chr;

        if ( $break eq "\xF7" ) {    # division sign
            if ( $word ne '' ) {
                push( @words, $word );
                $word = '';
            }

            my $wb = $wb->lookup($code);
            $word = $chr if $wb >= 1 && $wb <= 6;
        }
        elsif ( $break eq "\xD7" ) {    # multiplication sign
            $word .= $chr if $word ne '';
        }
        else {
            die("invalid break character '$break'");
        }
    }

    push( @words, $word ) if $word ne '';

    push(
        @tests,
        {   text  => $text,
            words => \@words,
        }
    );
}

close($in_file);

open( $out_file, '>', $tests_filename )
    or die("$tests_filename: $!\n");
print $out_file ( JSON->new->utf8->pretty->encode( \@tests ) );
close($out_file);

__DATA__
/*

This file is generated with devel/bin/gen_word_break_data.pl. DO NOT EDIT!
The contents of this file are derived from the Unicode Character Database,
version 6.3.0, available from http://www.unicode.org/Public/6.3.0/ucd/.
The Unicode copyright and permission notice follows.

Copyright (c) 1991-2011 Unicode, Inc. All rights reserved. Distributed under
the Terms of Use in http://www.unicode.org/copyright.html.

Permission is hereby granted, free of charge, to any person obtaining a copy of
the Unicode data files and any associated documentation (the "Data Files") or
Unicode software and any associated documentation (the "Software") to deal in
the Data Files or Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, and/or sell copies
of the Data Files or Software, and to permit persons to whom the Data Files or
Software are furnished to do so, provided that (a) the above copyright
notice(s) and this permission notice appear with all copies of the Data Files
or Software, (b) both the above copyright notice(s) and this permission notice
appear in associated documentation, and (c) there is clear notice in each
modified Data File or in the Software as well as in the documentation
associated with the Data File(s) or Software that the data or software has been
modified.

THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD
PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN
THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE DATA FILES OR
SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
these Data Files or Software without prior written authorization of the
copyright holder.

*/

