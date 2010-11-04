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
use lib 'buildlib';

use Test::More tests => 16;
use Lucy::Test::TestUtils qw( test_analyzer );

my $source_text = 'Eats, shoots and leaves.';

my $case_folder = Lucy::Analysis::CaseFolder->new;
my $tokenizer   = Lucy::Analysis::Tokenizer->new;
my $stopalizer  = Lucy::Analysis::Stopalizer->new( language => 'en' );
my $stemmer     = Lucy::Analysis::Stemmer->new( language => 'en' );

my $polyanalyzer
    = Lucy::Analysis::PolyAnalyzer->new( analyzers => [], );
test_analyzer( $polyanalyzer, $source_text, [$source_text],
    'no sub analyzers' );

$polyanalyzer
    = Lucy::Analysis::PolyAnalyzer->new( analyzers => [$case_folder], );
test_analyzer(
    $polyanalyzer, $source_text,
    ['eats, shoots and leaves.'],
    'with CaseFolder'
);

$polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer ], );
test_analyzer(
    $polyanalyzer, $source_text,
    [ 'eats', 'shoots', 'and', 'leaves' ],
    'with Tokenizer'
);

$polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer, $stopalizer ], );
test_analyzer(
    $polyanalyzer, $source_text,
    [ 'eats', 'shoots', 'leaves' ],
    'with Stopalizer'
);

$polyanalyzer = Lucy::Analysis::PolyAnalyzer->new(
    analyzers => [ $case_folder, $tokenizer, $stopalizer, $stemmer, ], );
test_analyzer( $polyanalyzer, $source_text, [ 'eat', 'shoot', 'leav' ],
    'with Stemmer' );

ok( $polyanalyzer->get_analyzers(), "get_analyzers method" );
