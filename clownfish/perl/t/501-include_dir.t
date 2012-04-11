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

use Test::More tests => 25;

use Clownfish::CFC::Model::Hierarchy;
use Clownfish::CFC::Util qw( a_isa_b );
use File::Spec::Functions qw( catfile splitpath );
use Fcntl;
use File::Path qw( rmtree mkpath );

my $source = 't/cfsource';
my $ext    = 't/cfext';
my $dest   = 't/cfdest';

my $class_clash = 't/cfclash/class';
my $file_clash  = 't/cfclash/file';

# One source, one include

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($ext);
    is_deeply( $hierarchy->get_source_dirs, [ $ext ], "get_source_dirs" );

    $hierarchy->add_include_dir($source);
    is_deeply( $hierarchy->get_include_dirs, [ $source ], "get_include_dirs" );

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 4, "all classes" );
    my $num_included = 0;
    for my $class (@$classes) {
        die "not a Class" unless isa_ok( $class, "Clownfish::CFC::Model::Class" );

        my $expect;

        if ($class->get_class_name eq "Animal::Rottweiler") {
            $expect = 0;
            my $parent_name = $class->get_parent->get_class_name;
            is( $parent_name, "Animal::Dog", "parent of included class" );
        }
        else {
            $expect = 1;
        }

        my $included = $class->included ? 1 : 0;
        ++$num_included if $included;

        is( $included, $expect, "included" );
    }
    is( $num_included, 3, "included class count" );

    Clownfish::CFC::Model::Class->_clear_registry();
}

# Two sources

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($source);
    $hierarchy->add_source_dir($ext);
    is_deeply( $hierarchy->get_source_dirs, [ $source, $ext ], "get_source_dirs" );
    is_deeply( $hierarchy->get_include_dirs, [], "get_include_dirs" );

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 4, "all classes" );
    for my $class (@$classes) {
        die "not a Class" unless isa_ok( $class, "Clownfish::CFC::Model::Class" );

        if ($class->get_class_name eq "Animal::Rottweiler") {
            my $parent_name = $class->get_parent->get_class_name;
            is( $parent_name, "Animal::Dog", "parent of class from second source" );
        }
    }

    Clownfish::CFC::Model::Class->_clear_registry();
}

# Name clashes

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($source);
    $hierarchy->add_source_dir($class_clash);

    eval { $hierarchy->build; };

    like( $@, qr/Conflict with existing class Animal::Dog/, "source/source class name clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($source);
    $hierarchy->add_source_dir($file_clash);

    eval { $hierarchy->build; };

    like( $@, qr/File Animal\/Dog.cfh already registered/, "source/source filename clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($source);
    $hierarchy->add_include_dir($class_clash);

    eval { $hierarchy->build; };

    like( $@, qr/Conflict with existing class Animal::Dog/, "source/include class name clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest);

    $hierarchy->add_source_dir($source);
    $hierarchy->add_include_dir($file_clash);

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 3, "source/include filename clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
}

