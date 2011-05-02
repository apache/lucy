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
use Lucy::Test;

package MySchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self = shift->SUPER::new(@_);
    my $tokenizer = Lucy::Analysis::RegexTokenizer->new( pattern => '\S+' );
    my $wordchar_tokenizer
        = Lucy::Analysis::RegexTokenizer->new( pattern => '\w+', );
    my $stopfilter
        = Lucy::Analysis::SnowballStopFilter->new( stoplist => { x => 1 } );
    my $fancy_analyzer = Lucy::Analysis::PolyAnalyzer->new(
        analyzers => [ $wordchar_tokenizer, $stopfilter, ], );

    my $plain = Lucy::Plan::FullTextType->new( analyzer => $tokenizer );
    my $fancy = Lucy::Plan::FullTextType->new( analyzer => $fancy_analyzer );
    $self->spec_field( name => 'plain', type => $plain );
    $self->spec_field( name => 'fancy', type => $fancy );
    return $self;
}

package main;

# Build index.
my $doc_set = Lucy::Test::TestUtils::doc_set()->to_perl;
my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = MySchema->new;
my $indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
);
for my $content_string (@$doc_set) {
    $indexer->add_doc(
        {   plain => $content_string,
            fancy => $content_string,
        }
    );
}
$indexer->commit;

Lucy::Test::Search::TestQueryParserSyntax::run_tests($folder);

