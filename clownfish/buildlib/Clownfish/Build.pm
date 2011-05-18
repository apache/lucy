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

package Clownfish::Build;
use base qw( Module::Build );

sub extra_ccflags {
    my $self = shift;
    my $extra_ccflags = defined $ENV{CFLAGS} ? "$ENV{CFLAGS} " : "";
    my $gcc_version 
        = $ENV{REAL_GCC_VERSION}
        || $self->config('gccversion')
        || undef;
    if ( defined $gcc_version ) {
        $gcc_version =~ /^(\d+(\.\d+))/
            or die "Invalid GCC version: $gcc_version";
        $gcc_version = $1;
    }

    if ( defined $ENV{LUCY_DEBUG} ) {
        if ( defined $gcc_version ) {
            $extra_ccflags .= "-DLUCY_DEBUG ";
            $extra_ccflags
                .= "-DPERL_GCC_PEDANTIC -std=gnu99 -pedantic -Wall ";
            $extra_ccflags .= "-Wextra " if $gcc_version >= 3.4;    # correct
            $extra_ccflags .= "-Wno-variadic-macros "
                if $gcc_version > 3.4;    # at least not on gcc 3.4
        }
    }

    if ( $ENV{LUCY_VALGRIND} and defined $gcc_version ) {
        $extra_ccflags .= "-fno-inline-functions ";
    }

    # Compile as C++ under MSVC.
    if ( $self->config('cc') eq 'cl' ) {
        $extra_ccflags .= '/TP ';
    }

    if ( defined $gcc_version ) {
        # Tell GCC explicitly to run with maximum options.
        if ( $extra_ccflags !~ m/-std=/ ) {
            $extra_ccflags .= "-std=gnu99 ";
        }
        if ( $extra_ccflags !~ m/-D_GNU_SOURCE/ ) {
            $extra_ccflags .= "-D_GNU_SOURCE ";
        }
    }

    return $extra_ccflags;
}

sub new {
    my ( $class, %args ) = @_;
    return $class->SUPER::new(
        %args,
        recursive_test_files => 1,
        extra_compiler_flags => __PACKAGE__->extra_ccflags,
    );
}

1;

