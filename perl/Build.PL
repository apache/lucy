use 5.008003;
use strict;
use warnings;

use lib 'buildlib';
use Lucy::Build;

my $builder = Lucy::Build->new(
    module_name        => 'Lucy',
    license            => 'apache',
    dist_author        => 'Marvin Humphrey <marvin at rectangular dot com>',
    dist_version_from  => 'lib/Lucy.pm',
    dist_abstract_from => 'lib/Lucy.pm',
    create_makefile_pl => 'passthrough',
    build_requires     => {
        'ExtUtils::CBuilder' => 0.18,
        'ExtUtils::ParseXS'  => 2.16,
        'Devel::PPPort'      => 3.13,
    },
    add_to_cleanup => [ 'Lucy-*', 'MANIFEST.bak', 'perltidy.ERR' ],
);

$builder->create_build_script();

__END__


    /**
     * Copyright 2006-2009 The Apache Software Foundation
     *
     * Licensed under the Apache License, Version 2.0 (the "License");
     * you may not use this file except in compliance with the License.
     * You may obtain a copy of the License at
     *
     *     http://www.apache.org/licenses/LICENSE-2.0
     *
     * Unless required by applicable law or agreed to in writing, software
     * distributed under the License is distributed on an "AS IS" BASIS,
     * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
     * implied.  See the License for the specific language governing
     * permissions and limitations under the License.
     */

