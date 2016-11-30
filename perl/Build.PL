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
use File::Spec::Functions qw( catdir updir );
use Lucy::Build;

# Don't create FAIL reports from CPAN Testers that force parallel tests.
# See LUCY-260.
if ( $ENV{AUTOMATED_TESTING}
     && $ENV{HARNESS_OPTIONS} =~ /(?:^|:)j(\d*)/
     && ($1 || 9) > 1
   ) {
    print STDERR "Parallel testing is known to fail.\n";
    exit;
}

my $IS_CPAN_DIST = -e 'cfcore';
my $CORE_SOURCE_DIR;
my $TEST_SOURCE_DIR;
if ($IS_CPAN_DIST) {
    $CORE_SOURCE_DIR = 'cfcore';
    $TEST_SOURCE_DIR = 'cftest';
}
else {
    $CORE_SOURCE_DIR = catdir( updir(), 'core' );
    $TEST_SOURCE_DIR = catdir( updir(), 'test' );
}

my $builder = Lucy::Build->new(
    module_name => 'Lucy',
    license     => 'apache',
    dist_author =>
        'The Apache Lucy Project <dev at lucy dot apache dot org>',
    dist_version       => '0.6.0',
    requires           => {
        'perl'      => '5.8.3',
        'Clownfish' => 0.006000,
    },
    configure_requires => {
        'Module::Build'               => 0.280801,
        'Clownfish'                   => 0.006000,
        'Clownfish::CFC::Perl::Build' => 0.006000,
    },
    build_requires     => {
        'Module::Build'      => 0.280801,
        'ExtUtils::CBuilder' => 0.21,
        'ExtUtils::ParseXS'  => 3.00,
        'Devel::PPPort'      => 3.14,
        'Clownfish'          => 0.006000,
        'Clownfish::CFC'     => 0.006000,
    },
    no_index => {
        directory => [qw( buildlib devel sample )],
        package   => [qw( Lucy::Test )],
    },
    meta_merge => { keywords => [qw( search lucy lucene )], },
    meta_add   => {
        resources => {
            homepage   => 'http://lucy.apache.org',
            repository => 'https://git-wip-us.apache.org/repos/asf/lucy.git',
            bugtracker => 'https://issues.apache.org/jira/browse/LUCY',
        },
    },
    include_dirs => [ $CORE_SOURCE_DIR ],
    clownfish_params => {
        source => [ $CORE_SOURCE_DIR, $TEST_SOURCE_DIR ],
        modules => [
            {
                name          => 'Lucy',
                parcels       => [ 'Lucy' ],
                make_target   => 'core_objects',
                c_source_dirs => [ 'xs' ],
            },
            {
                name        => 'Lucy::Test',
                parcels     => [ 'TestLucy' ],
                make_target => 'test_objects',
            },
        ],
    },
    add_to_cleanup => [
        qw(
            Lucy-*
            MANIFEST.bak
            perltidy.ERR
            *.pdb
            *.manifest
            ),
    ],
);

$builder->create_build_script();

__END__
