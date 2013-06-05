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
use File::Find 'find';

my @modules;

# None for now -- until we remove a module.
my %excluded = map { ( $_ => 1 ) } qw();

find(
    {   no_chdir => 1,
        wanted   => sub {
            return unless $File::Find::name =~ /\.pm$/;
            push @modules, $File::Find::name;
            }
    },
    'lib'
);

plan( tests => scalar @modules );

for (@modules) {
    s/^.*?Clownfish/Clownfish/;
    s/\.pm$//;
    s/\W+/::/g;
    if ( $excluded{$_} ) {
        eval qq|use $_;|;
        like( $@, qr/removed|replaced|renamed/i,
            "Removed module '$_' throws error on load" );
    }
    else {
        use_ok($_);
    }
}

