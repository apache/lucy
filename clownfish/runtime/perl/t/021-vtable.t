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

package MyHash;
use base qw( Clownfish::Hash );

sub oodle { }

package main;

use Test::More tests => 5;

my $stringified;
my $storage = Clownfish::Hash->new;

{
    my $subclassed_hash = MyHash->new;
    $stringified = $subclassed_hash->to_string;

    isa_ok( $subclassed_hash, "MyHash", "Perl isa reports correct subclass" );

   # Store the subclassed object.  At the end of this block, the Perl object
   # will go out of scope and DESTROY will be called, but the Clownfish object
   # will persist.
    $storage->store( "test", $subclassed_hash );
}

my $resurrected = $storage->_fetch("test");

isa_ok( $resurrected, "MyHash", "subclass name survived Perl destruction" );
is( $resurrected->to_string, $stringified,
    "It's the same Hash from earlier (though a different Perl object)" );

my $booga = Clownfish::String->new("booga");
$resurrected->store( "ooga", $booga );

is( $resurrected->fetch("ooga"),
    "booga", "subclassed object still performs correctly at the C level" );

my $methods = Clownfish::VTable::_fresh_host_methods('MyHash');
is_deeply( $methods->to_perl, ['oodle'], "fresh_host_methods" );

