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

sub doc_max  { 100 }
sub doc_freq {   1 }

package MyQuery;
use base qw( Lucy::Search::Query );

our $subordinate_arg;

sub make_compiler {
    my ( $self, %args ) = @_;
    $subordinate_arg = delete($args{subordinate});
    return MyCompiler->new( %args, parent => $self );
}

package MyCompiler;
use base qw( Lucy::Search::Compiler );

our $similarity_arg;

sub new {
    my ( $self, %args ) = @_;
    return $self->SUPER::new(%args);
}

sub _normalize {
    my ( $self, $sim ) = @_;
    $similarity_arg = $sim;
}

package QueryWithoutMakeCompiler;
use base qw( Lucy::Search::Query );

package main;
use Test::More tests => 8;

my $schema   = Lucy::Plan::Schema->new;
my $sim      = $schema->get_similarity;
my $searcher = MockSearcher->new( schema => $schema );
my $query;
my $compiler;

$query = MyQuery->new;
$query->set_boost(123.0);
$compiler = $query->make_root_compiler($searcher);
ok( $MyQuery::subordinate_arg, "make_compiler receives true subordinate arg" );

is( $compiler->get_boost, 123.0, "get_boost" );
is( $compiler->get_weight, 123.0, "get_weight" );
is( ${ $compiler->get_parent }, $$query, "get_parent" );
is( ${ $compiler->get_similarity }, $$sim, "get_similarity" );

$MyCompiler::similarity_arg = undef;
$compiler->normalize;
is( $$MyCompiler::similarity_arg, $$sim, "normalize without sim" );

eval {
    QueryWithoutMakeCompiler->new->make_root_compiler($searcher);
};
like( $@, qr('Make_Compiler' not defined by QueryWithoutMakeCompiler),
      "Subclassing Query without overriding make_compiler throws" );

$query = Lucy::Search::TermQuery->new( field => 'content', term => 'foo' );
$compiler = $query->make_compiler(
    searcher => $searcher,
    boost    => 64.0,
);
# normalized_weight == idf == 1 + log(100 / (1 + 1))
ok( $compiler->get_weight > 1, "make_compiler normalizes Compiler" );

