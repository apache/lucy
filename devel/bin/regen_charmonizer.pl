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

use FindBin qw( $Bin );
use File::Spec::Functions qw( catdir catfile updir );

# Execute from the root of the Lucy repository.
chdir catdir( $Bin, updir(), updir() );

my $CHAZ_DIR;

if (@ARGV > 0) {
    $CHAZ_DIR = $ARGV[0];
}
else {
    # Assume that Charmonizer is in '../charmonizer'.
    $CHAZ_DIR = catdir( updir(), 'charmonizer' );
}

my $MELD_EXE = catfile( $CHAZ_DIR, 'buildbin', 'meld.pl' );
die("Couldn't find meld.pl at $MELD_EXE")
    if !-e $MELD_EXE;

my $main = catfile(qw( common charmonizer.main ));
my $out  = $main;
$out =~ s/\.main/.c/ or die "no match";
unlink $out;
system( $MELD_EXE, '--probes=', "--files=$main", "--out=$out" );

