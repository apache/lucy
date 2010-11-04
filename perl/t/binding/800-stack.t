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

package MockSearcher;
use base qw( Lucy::Search::Searcher );

package MyQuery;
use base qw( Lucy::Search::Query );

sub make_compiler {
    my $self = shift;
    return MyCompiler->new( @_, parent => $self );
}

package MyCompiler;
use base qw( Lucy::Search::Compiler );

sub apply_norm_factor {
    my ( $self, $factor ) = @_;
    $self->SUPER::apply_norm_factor($factor);
}

package main;
use Test::More tests => 1;

my $q = Lucy::Search::ORQuery->new;
for ( 1 .. 50 ) {
    my @kids = ( $q, ( MyQuery->new ) x 10 );
    $q = Lucy::Search::ORQuery->new( children => \@kids );
}
my $searcher = MockSearcher->new( schema => Lucy::Plan::Schema->new );
my $compiler = $q->make_compiler( searcher => $searcher );

pass("Made it through deep recursion with multiple stack reallocations");

