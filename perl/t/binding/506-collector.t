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

package EvensOnlyCollector;
use base qw( Lucy::Search::Collector );

our %doc_ids;

sub new {
    my $self = shift->SUPER::new;
    $doc_ids{$$self} = [];
    return $self;
}

sub collect {
    my ( $self, $doc_id ) = @_;
    if ( $doc_id % 2 == 0 ) {
        push @{ $doc_ids{$$self} }, $doc_id;
    }
}

sub get_doc_ids { $doc_ids{ ${ +shift } } }

sub DESTROY {
    my $self = shift;
    delete $doc_ids{$$self};
    $self->SUPER::DESTROY;
}

package main;

use Test::More tests => 1;
use Lucy::Test;
use LucyX::Search::MockMatcher;

my $collector = EvensOnlyCollector->new;
my $matcher
    = LucyX::Search::MockMatcher->new( doc_ids => [ 1, 5, 10, 1000 ], );
$collector->set_matcher($matcher);
while ( my $doc_id = $matcher->next ) {
    $collector->collect($doc_id);
}
is_deeply(
    $collector->get_doc_ids,
    [ 10, 1000 ],
    "Collector can be subclassed"
);

