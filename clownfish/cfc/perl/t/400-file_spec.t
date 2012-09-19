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

use Clownfish::CFC::Model::FileSpec;

my %args = (
    source_dir  => 'Clownfish/_include',
    path_part   => 'Stuff/Thing',
);
my $file_spec;

$file_spec = Clownfish::CFC::Model::FileSpec->new(%args);
is( $file_spec->get_source_dir, "Clownfish/_include", "get_source_dir" );
is( $file_spec->get_path_part, "Stuff/Thing", "get_path_part" );
ok( !$file_spec->included, "included default" );

$file_spec = Clownfish::CFC::Model::FileSpec->new(
    %args,
    is_included => 1,
);
ok( $file_spec->included, "included" );

