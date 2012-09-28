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

use 5.008003;
use strict;
use warnings;
use lib 'buildlib';
use File::Spec::Functions qw( catdir );
use Clownfish::Build;

my @BASE_PATH        = Clownfish::Build->cf_base_path;
my $CORE_SOURCE_DIR  = catdir( @BASE_PATH, 'core' );
my $XS_SOURCE_DIR    = 'xs';

my $builder = Clownfish::Build->new(
    module_name => 'Clownfish',
    license     => 'apache',
    dist_author =>
        'The Apache Lucy Project <dev at lucy dot apache dot org>',
    dist_version       => '0.3.0',
    dist_abstract      => 'Clownfish Runtime',
    requires           => { 'perl' => '5.8.3', },
    configure_requires => { 'Module::Build' => 0.280801 },
    build_requires     => {
        'Module::Build'      => 0.280801,
        'ExtUtils::CBuilder' => 0.21,
        'ExtUtils::ParseXS'  => 2.18,
        'Devel::PPPort'      => 3.13,
    },
    meta_merge => { keywords => [qw( clownfish )], },
    meta_add   => {
        resources => {
            homepage   => 'http://lucy.apache.org',
            repository => 'http://svn.apache.org/repos/asf/lucy',
            bugtracker => 'https://issues.apache.org/jira/browse/LUCY',
        },
    },
    include_dirs => [
        $CORE_SOURCE_DIR,
        $XS_SOURCE_DIR,
    ],
    clownfish_params => {
        source => [
            $CORE_SOURCE_DIR,
            $XS_SOURCE_DIR,
        ],
    },
    add_to_cleanup => [
        qw(
            Clownfish-*
            MANIFEST.bak
            perltidy.ERR
            *.pdb
            *.manifest
            ),
    ],
);

$builder->create_build_script();

__END__
