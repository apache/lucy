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
use lib 'buildlib';

use Test::More tests => 6;
use Storable qw( freeze thaw );
use Lucy::Test::TestUtils qw( utf8_test_strings );

my ( $smiley, $not_a_smiley, $frowny ) = utf8_test_strings();

my $charbuf = Lucy::Object::CharBuf->new($smiley);
isa_ok( $charbuf, "Lucy::Object::CharBuf" );
is( $charbuf->to_perl, $smiley, "round trip UTF-8" );

$charbuf = Lucy::Object::CharBuf->new($smiley);
my $dupe = thaw( freeze($charbuf) );
isa_ok( $dupe, "Lucy::Object::CharBuf",
    "thaw/freeze produces correct object" );
is( $dupe->to_perl, $charbuf->to_perl, "freeze/thaw" );

my $clone = $charbuf->clone;
is( $clone->to_perl, Lucy::Object::CharBuf->new($smiley)->to_perl, "clone" );

my $ram_file = Lucy::Store::RAMFile->new;
my $outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Lucy->error;
$charbuf->serialize($outstream);
$outstream->close;
my $instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Lucy->error;
my $deserialized = Lucy::Object::CharBuf->deserialize($instream);
is_deeply( $charbuf->to_perl, $deserialized->to_perl,
    "serialize/deserialize" );

