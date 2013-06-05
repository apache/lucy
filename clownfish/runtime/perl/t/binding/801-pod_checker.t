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

use Test::More;
use Pod::Checker;
BEGIN {
    if ( $] < 5.010 ) {
        plan( 'skip_all', "Old Pod::Checker is buggy" );
    }
    else {
        plan('no_plan');
    }
}

use File::Find qw( find );

my @filepaths;
find(
    {   no_chdir => 1,
        wanted   => sub {
            return unless $File::Find::name =~ /\.(pm|pod)$/;
            push @filepaths, $File::Find::name;
            }
    },
    'lib'
);

for my $path (@filepaths) {
    my $pod_ok = podchecker( $path, undef, -warnings => 0 );
    if ( $pod_ok == -1 ) {
        # No POD.
    }
    elsif ( $pod_ok == 0 ) {
        pass("POD ok for '$path'");
    }
    else {
        fail("Bad POD for '$path'");
    }
}
