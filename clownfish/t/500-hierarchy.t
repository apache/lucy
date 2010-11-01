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

use Test::More tests => 18;

use Clownfish::Hierarchy;
use Clownfish::Util qw( a_isa_b );
use File::Spec::Functions qw( catfile splitpath );
use Fcntl;
use File::Path qw( rmtree mkpath );

my %args = (
    source => 't/cfsource',
    dest   => 't/cfdest',
);

# Clean up.
rmtree( $args{dest} );

eval { my $death = Clownfish::Hierarchy->new( %args, extra_arg => undef ) };
like( $@, qr/extra_arg/, "Extra arg kills constructor" );

my $hierarchy = Clownfish::Hierarchy->new(%args);
isa_ok( $hierarchy, "Clownfish::Hierarchy" );
is( $hierarchy->get_source, $args{source}, "get_source" );
is( $hierarchy->get_dest,   $args{dest},   "get_dest" );

$hierarchy->build;

my @files = $hierarchy->files;
is( scalar @files, 3, "recursed and found all three files" );
my %files;
for my $file (@files) {
    die "not a File" unless isa_ok( $file, "Clownfish::File" );
    ok( !$file->get_modified, "start off not modified" );
    my ($class) = grep { a_isa_b( $_, "Clownfish::Class" ) } $file->blocks;
    die "no class" unless $class;
    $files{ $class->get_class_name } = $file;
}
my $animal = $files{'Animal'}       or die "No Animal";
my $dog    = $files{'Animal::Dog'}  or die "No Dog";
my $util   = $files{'Animal::Util'} or die "No Util";

my @classes = $hierarchy->ordered_classes;
is( scalar @classes, 3, "all classes" );
for my $class (@classes) {
    die "not a Class" unless isa_ok( $class, "Clownfish::Class" );
}

# Generate fake C files, with times set to one second ago.
my $one_second_ago = time() - 1;
for my $file (@files) {
    my $h_path = $file->h_path( $args{dest} );
    my ( undef, $dir, undef ) = splitpath($h_path);
    mkpath($dir);
    sysopen( my $fh, $h_path, O_CREAT | O_EXCL | O_WRONLY )
        or die "Can't open '$h_path': $!";
    print $fh "#include <stdio.h>\n";    # fake content.
    close $fh or die "Can't close '$h_path': $!";
    utime( $one_second_ago, $one_second_ago, $h_path )
        or die "utime failed for '$h_path': $!";
}

my $path_to_animal_cf = $animal->cfh_path( $args{source} );
utime( undef, undef, $path_to_animal_cf )
    or die "utime for '$path_to_animal_cf' failed";    # touch

$hierarchy->propagate_modified;

ok( $animal->get_modified, "Animal modified" );
ok( $dog->get_modified, "Parent's modification propagates to child's file" );
ok( !$util->get_modified, "modification doesn't propagate to inert class" );

# Clean up.
rmtree( $args{dest} );

