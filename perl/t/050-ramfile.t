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

use Test::More tests => 8;
use Lucy::Test;

my ( $ram_file, $outstream, $instream, $foo );

$ram_file = Lucy::Store::RAMFile->new;
$outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Clownfish->error;
$outstream->print("foo");
$outstream->flush;
is( $ram_file->get_contents, "foo", '$ramfile->get_contents' );

my $long_string = 'a' x 5000;
$outstream->print($long_string);
$outstream->flush;

is( $ram_file->get_contents, "foo$long_string",
    "store a string spread out over several buffers" );

$instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Clownfish->error;
$instream->read( $foo, 3 );
is( $foo, 'foo', "instream reads ramfile properly" );

my $long_dupe;
$instream->read( $long_dupe, 5000 );
is( $long_dupe, $long_string, "read string spread out over several buffers" );

eval { my $blah; $instream->read( $blah, 3 ); };
like( $@, qr/EOF/, "reading past EOF throws an error" );

$ram_file = Lucy::Store::RAMFile->new;
$outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Clownfish->error;
my $BUF_SIZE  = Lucy::Store::FileHandle::_BUF_SIZE();
my $rep_count = $BUF_SIZE - 1;
$outstream->print( 'a' x $rep_count );
$outstream->print('foo');
$outstream->close;
$instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Clownfish->error;
$instream->read( $long_dupe, $rep_count );
undef $foo;
$instream->read( $foo, 3 );
is( $foo, 'foo', "read across buffer boundary " );

$ram_file = Lucy::Store::RAMFile->new;
$outstream = Lucy::Store::OutStream->open( file => $ram_file )
    or die Clownfish->error;
$outstream->print( 'a' x 1024 );
$outstream->print('foo');
$outstream->close;

$instream = Lucy::Store::InStream->open( file => $ram_file )
    or die Clownfish->error;
$instream->seek(1024);
undef $foo;
$instream->read( $foo, 3 );
is( $foo, 'foo', "InStream seek" );

my $dupe = $instream->reopen(
    filename => 'foo',
    offset   => 1023,
    len      => 4
);
undef $foo;
$dupe->read( $foo, 4 );
is( $foo, 'afoo', "reopened instream" );

