#!/usr/bin/perl

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
use File::Spec::Functions qw( catfile rel2abs tmpdir );
use File::Find qw( find );
use File::Temp qw( mktemp );

#insert code here


__END__

=head1 NAME

generate_charmonizer_makefiles.pl

=head1 SYNOPSIS

    generate_charmonizer_makefiles.pl - keeps the Makefiles in sync with the live tree.

=head1 DESCRIPTION

    Insert verbose description.

