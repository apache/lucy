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
use Test::More tests => 1;
use Lucy::Test;

my $schema = Lucy::Plan::Schema->new;
$schema->spec_field(
    name => 'content',
    type => Lucy::Plan::FullTextType->new(
        analyzer      => Lucy::Analysis::RegexTokenizer->new,
        highlightable => 1,
    ),
);
my $folder  = Lucy::Store::RAMFolder->new;
my $indexer = Lucy::Index::Indexer->new(
    schema => $schema,
    index  => $folder,
    create => 1,
);
$indexer->add_doc(
    doc => {
        content => <<'EOF',
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla NNN bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla MMM bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla bla.
EOF
    },
);
$indexer->commit();

my $searcher    = Lucy::Search::IndexSearcher->new( index => $folder, );
my $query       = 'NNN MMM';
my $highlighter = Lucy::Highlight::Highlighter->new(
    searcher => $searcher,
    query    => $query,
    field    => 'content'
);
my $hits    = $searcher->hits( query => $query, );
my $hit     = $hits->next();
my $excerpt = $highlighter->create_excerpt($hit);
like( $excerpt, qr/(NNN|MMM)/, "Sentence boundary algo doesn't chop terms" );

