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

use Test::More tests => 6;

use Clownfish::CFC::Model::Hierarchy;
use File::Spec::Functions qw( catdir catfile splitpath );
use File::Path qw( rmtree );

my $base_dir        = catdir(qw( t cfbase ));
my $ext_dir         = catdir(qw( t cfext ));
my $dest_dir        = catdir(qw( t cfdest ));
my $class_clash_dir = catdir(qw( t cfclash class ));
my $file_clash_dir  = catdir(qw( t cfclash file ));

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($base_dir);
    $hierarchy->add_source_dir($class_clash_dir);

    eval { $hierarchy->build; };

    like( $@, qr/Conflict with existing class Animal::Dog/,
          "source/source class name clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($base_dir);
    $hierarchy->add_source_dir($file_clash_dir);

    eval { $hierarchy->build; };

    my $filename = catfile(qw( Animal Dog.cfh ));
    like( $@, qr/File \Q$filename\E already registered/,
          "source/source filename clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($base_dir);
    $hierarchy->add_include_dir($class_clash_dir);

    eval { $hierarchy->build; };

    like( $@, qr/Class .* from include dir .* parcel .* from source dir/,
          "source/include class name clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($base_dir);
    $hierarchy->add_include_dir($file_clash_dir);

    $hierarchy->build;

    my $classes = $hierarchy->ordered_classes;
    is( scalar @$classes, 3, "source/include filename clash" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

# Parcel/class include mismatch

my $foo_dir = catdir(qw( t cfclash foo ));
my $bar_dir = catdir(qw( t cfclash bar ));

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($foo_dir);
    $hierarchy->add_include_dir($bar_dir);

    eval { $hierarchy->build; };

    like( $@, qr/Class .* from include dir .* parcel .* from source dir/,
          "included class with source parcel" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

{
    my $hierarchy = Clownfish::CFC::Model::Hierarchy->new(dest => $dest_dir);

    $hierarchy->add_source_dir($bar_dir);
    $hierarchy->add_include_dir($foo_dir);

    eval { $hierarchy->build; };

    like( $@, qr/Class .* from source dir .* parcel .* from include dir/,
          "source class with included parcel" );

    Clownfish::CFC::Model::Class->_clear_registry();
    Clownfish::CFC::Model::Parcel->reap_singletons();
}

# Clean up.
rmtree($dest_dir);

