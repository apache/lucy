#!/usr/local/bin/perl

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

use lib '../devel/benchmarks/indexers';
use lib 'devel/benchmarks/indexers';

use Getopt::Long;
use Cwd qw( getcwd );
use BenchmarkingIndexer;

# Index all docs and run one iter unless otherwise spec'd.
my ( $num_reps, $max_to_index, $increment, $store, $build_index );
GetOptions(
    'reps=s'        => \$num_reps,
    'docs=s'        => \$max_to_index,
    'increment=s'   => \$increment,
    'store=s'       => \$store,
    'build_index=s' => \$build_index,
);
$num_reps = 1 unless defined $num_reps;

my $bencher = BenchmarkingIndexer::Lucy->new(
    docs      => $max_to_index,
    increment => $increment,
    store     => $store,
);

if ($build_index) {
    my ( $count, $secs ) = $bencher->build_index;
    print "docs: $count elapsed: $secs\n";
    exit;
}
else {
    $bencher->start_report;

    my @times;
    for my $rep ( 1 .. $num_reps ) {

        # Spawn an index-building child process.
        my $command = "$^X ";
        # Try to figure out if this program was called with -Mblib.
        for (@INC) {
            next unless /\bblib\b/;
            # Propagate -Mblib to the child.
            $command .= "-Mblib ";
            last;
        }
        $command .= "$0 --build_index=1 ";
        $command .= "--docs=$max_to_index " if $max_to_index;
        $command .= "--store=$store " if $store;
        $command .= "--increment=$increment " if $increment;
        my $output = `$command`;

        # Extract elapsed time from the output of the child.
        $output =~ /^docs: (\d+) elapsed: ([\d.]+)/
            or die "no match: '$output'";
        my $docs = $1;
        my $secs = $2;
        push @times, $secs;

        $bencher->print_interim_report(
            rep   => $rep,
            secs  => $secs,
            count => $docs,
        );
    }

    $bencher->print_final_report( \@times );
}

