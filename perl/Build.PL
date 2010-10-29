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

my $conflict = 0;
my $existing;

BEGIN {
    eval 'use KinoSearch;';
    if ( !$@ ) {
        $existing = eval $KinoSearch::VERSION;
        if ( $existing < 0.20 ) {
            $conflict = 1;
        }
    }
}

use lib 'buildlib';
use KinoSearch::Build;

if ( $conflict and !$ENV{KINO_DEBUG} ) {
    print <<END_WARNING;

*** WARNING ***
An existing installation of KinoSearch version $existing has been detected.
This release breaks backwards compatibility with the 0.1x branch.  Both the
file format and the API have changed.

END_WARNING

    my $response = Module::Build->y_n("Continue? (y/n) ");
    exit unless $response;
}

my $builder = KinoSearch::Build->new(
    module_name       => 'KinoSearch',
    license           => 'apache',
    dist_author       => 'Marvin Humphrey <marvin at rectangular dot com>',
    dist_version_from => 'lib/KinoSearch.pm',
    requires          => {
        'Lingua::Stem::Snowball' => 0.952,
        'Lingua::StopWords'      => 0.09,
        'Parse::RecDescent'      => 0,
        'JSON::XS'               => 1.53,
    },
    build_requires => {
        'ExtUtils::CBuilder' => 0.18,
        'ExtUtils::ParseXS'  => 2.16,
        'Devel::PPPort'      => 3.13,
    },
    meta_merge         => { keywords => [qw( search lucene )], },
    meta_add           => {
        resources => {
            homepage   => 'http://www.rectangular.com/kinosearch',
            repository => 'http://www.rectangular.com/svn/kinosearch/trunk',
        },
    },
    add_to_cleanup => [
        qw(
            KinoSearch-*
            MANIFEST.bak
            perltidy.ERR
            *.pdb
            *.manifest
            ),
    ],
);

$builder->create_build_script();

__END__

