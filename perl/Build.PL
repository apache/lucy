use 5.008003;
use strict;
use warnings;

use lib 'buildlib';
use Lucy::Build;
use File::Spec::Functions qw( catdir updir );


=for Rationale

When the distribution tarball for the Perl binding of Lucy is built, c_src/,
c_test/, and any other needed files/directories are copied into the perl/
directory within the main Lucy directory.  Then the distro is built from the
contents of the perl/ directory, leaving out all the files in ruby/, etc.
However, during development, the files are access from their original
locations.

=cut

my $c_src = -e 'c_src' ? 'c_src' : catdir( updir(), 'c_src' );

my $builder = Lucy::Build->new(
    module_name        => 'Lucy',
    license            => 'apache',
    dist_author        => 'Marvin Humphrey <marvin at rectangular dot com>',
    dist_version_from  => 'lib/Lucy.pm',
    dist_abstract_from => 'lib/Lucy.pm',
    create_makefile_pl => 'passthrough',
    build_requres      => {
        'ExtUtils::CBuilder' => 0.18,
        'ExtUtils::ParseXS'  => 2.16,
    },
    add_to_cleanup => [
        'Lucy-*',       'typemap', 'MANIFEST.bak', 'lastmod',
        'perltidy.ERR', '*.o',
    ],
);

$builder->create_build_script();

__END__


    /**
     * Copyright 2006 The Apache Software Foundation
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

