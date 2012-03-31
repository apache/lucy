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

use Test::More tests => 17;

use Clownfish::CFC::Model::File;
use Clownfish::CFC::Parser;

my $parser = Clownfish::CFC::Parser->new;

my $parcel_declaration = "parcel Stuff;";
my $class_content      = qq|
    class Stuff::Thing {
        Foo *foo;
        Bar *bar;
    }
|;
my $c_block = "__C__\nint foo;\n__END_C__\n";

my $file
    = $parser->_parse_file( "$parcel_declaration\n$class_content\n$c_block",
    '.', 'Stuff/Thing', 0 );

is( $file->get_path_part, "Stuff/Thing", "get_path_part" );

my $guard_name = $file->guard_name;
is( $guard_name, "H_STUFF_THING", "guard_name" );
like( $file->guard_start, qr/$guard_name/, "guard_start" );
like( $file->guard_close, qr/$guard_name/,
    "guard_close includes guard_name" );

ok( !$file->get_modified, "modified false at start" );
$file->set_modified(1);
ok( $file->get_modified, "set_modified, get_modified" );

my $path_sep = $^O =~ /^mswin/i ? '\\' : '/';
my $path_to_stuff_thing = join( $path_sep, qw( path to Stuff Thing ) );
is( $file->cfh_path('path/to'), "$path_to_stuff_thing.cfh", "cfh_path" );
is( $file->c_path('path/to'),   "$path_to_stuff_thing.c",   "c_path" );
is( $file->h_path('path/to'),   "$path_to_stuff_thing.h",   "h_path" );

my $classes = $file->classes;
is( scalar @$classes, 1, "classes() filters blocks" );
my $class = $classes->[0];
my ( $foo, $bar ) = @{ $class->member_vars };
is( $foo->get_type->get_specifier,
    'stuff_Foo', 'file production picked up parcel def' );
is( $bar->get_type->get_specifier, 'stuff_Bar', 'parcel def is sticky' );

my $blocks = $file->blocks;
is( scalar @$blocks, 3, "all three blocks" );
isa_ok( $blocks->[0], "Clownfish::CFC::Model::Parcel" );
isa_ok( $blocks->[1], "Clownfish::CFC::Model::Class" );
isa_ok( $blocks->[2], "Clownfish::CFC::Model::CBlock" );

$file = $parser->_parse_file( $class_content, '.', 'Stuff/Thing', 0 );
($class) = @{ $file->classes };
( $foo, $bar ) = @{ $class->member_vars };
is( $foo->get_type->get_specifier, 'Foo', 'file production resets parcel' );

