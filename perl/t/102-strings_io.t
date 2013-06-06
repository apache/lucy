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

use Test::More tests => 3;
use Lucy::Test;

my ( @items, $packed, $template, $buf, $file, $out, $in, $correct );

$file = Lucy::Store::RAMFile->new;
$out = Lucy::Store::OutStream->open( file => $file )
    or die Clownfish->error;
$out->write_c64(10000);
$out->close;
$in = Lucy::Store::InStream->open( file => $file )
    or die Clownfish->error;
$in->read_raw_c64($buf);
$correct = $file->get_contents;
is( $buf, $correct, "read_raw_c64" );

$file = Lucy::Store::RAMFile->new;
$out = Lucy::Store::OutStream->open( file => $file )
    or die Clownfish->error;
$out->print("mint");
$out->close;
$buf = "funny";
$in = Lucy::Store::InStream->open( file => $file )
    or die Clownfish->error;
$in->read( $buf, 1 );
is( $buf, "munny", 'read' );

$file = Lucy::Store::RAMFile->new;
$out = Lucy::Store::OutStream->open( file => $file )
    or die Clownfish->error;
$out->print("cute");
$out->close;
$in = Lucy::Store::InStream->open( file => $file )
    or die Clownfish->error;
$buf = "buzz";
$in->read( $buf, 3, 4 );
is( $buf, "buzzcut", 'read with offset' );
