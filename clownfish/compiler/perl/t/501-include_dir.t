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

use Test::More tests => 24;

use Clownfish::CFC::Model::Hierarchy;
use File::Spec::Functions qw( catdir catfile splitpath );
use File::Path qw( rmtree );

my $base_dir = catdir(qw( t cfbase ));
my $ext_dir  = catdir(qw( t cfext ));
my $dest_dir = catdir(qw( t cfdest ));

# One source, one include

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($ext_dir);
    is_deeply( $hierarchy->get_source_dirs, [ $ext_dir ], "get_source_dirs" );

    $hierarchy->add_include_dir($base_dir);
    is_deeply( $hierarchy->get_include_dirs, [ $base_dir ],
               "get_include_dirs" );

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 5, "all classes" );
    my $num_included = 0;
    for my $class (@$classes) {
        die "not a Class"
            unless isa_ok( $class, "Clownfish::CFC::Model::Class" );

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
    is( $num_included, 4, "included class count" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

# Two sources

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($base_dir);
    $hierarchy->add_source_dir($ext_dir);
    is_deeply( $hierarchy->get_source_dirs, [ $base_dir, $ext_dir ],
               "get_source_dirs" );
    is_deeply( $hierarchy->get_include_dirs, [], "get_include_dirs" );

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 5, "all classes" );
    for my $class (@$classes) {
        die "not a Class" unless isa_ok( $class, "Clownfish::CFC::Model::Class" );

        if ($class->get_class_name eq "Animal::Rottweiler") {
            my $parent_name = $class->get_parent->get_class_name;
            is( $parent_name, "Animal::Dog", "parent of class from second source" );
        }
    }

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

# Clean up.
rmtree($dest_dir);

