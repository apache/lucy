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

use Test::More tests => 4;
use Storable qw( nfreeze thaw );
use KinoSearch::Test;
use KinoSearch qw( to_perl to_clownfish );

my $hash = KinoSearch::Object::Hash->new( capacity => 10 );
$hash->store( "foo", KinoSearch::Object::CharBuf->new("bar") );
$hash->store( "baz", KinoSearch::Object::CharBuf->new("banana") );

ok( !defined( $hash->fetch("blah") ),
    "fetch for a non-existent key returns undef" );

my $frozen = nfreeze($hash);
my $thawed = thaw($frozen);
is_deeply( $thawed->to_perl, $hash->to_perl, "freeze/thaw" );

my $ram_file = KinoSearch::Store::RAMFile->new;
my $outstream = KinoSearch::Store::OutStream->open( file => $ram_file )
    or die KinoSearch->error;
$hash->serialize($outstream);
$outstream->close;
my $instream = KinoSearch::Store::InStream->open( file => $ram_file )
    or die KinoSearch->error;
my $deserialized = $hash->deserialize($instream);
is_deeply( $hash->to_perl, $deserialized->to_perl, "serialize/deserialize" );

my %hash_with_utf8_keys = ( "\x{263a}" => "foo" );
my $round_tripped = to_perl( to_clownfish( \%hash_with_utf8_keys ) );
is_deeply( $round_tripped, \%hash_with_utf8_keys,
    "Round trip conversion of hash with UTF-8 keys" );
