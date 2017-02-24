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

use Test::More tests => 3;
use Lucy;

package TestAnalyzer;
use base qw( Lucy::Analysis::Analyzer );
sub transform { $_[1] }
sub equals { $_[1]->isa(__PACKAGE__) }

package main;
use Encode qw( _utf8_on );
use Lucy::Test;

sub new_schema {
    my $schema     = Lucy::Plan::Schema->new;
    my $analyzer   = TestAnalyzer->new;
    my $fulltext   = Lucy::Plan::FullTextType->new( analyzer => $analyzer );
    my $not_stored = Lucy::Plan::FullTextType->new(
        analyzer => $analyzer,
        stored   => 0,
    );
    $schema->spec_field( name => 'text',     type => $fulltext );
    $schema->spec_field( name => 'unstored', type => $not_stored );
    $schema->spec_field( name => 'empty',    type => $fulltext );
    return $schema;
}

# This valid UTF-8 string includes skull and crossbones, null byte -- however,
# the binary value is not flagged as UTF-8.
my $bin_val = my $val = "a b c \xe2\x98\xA0 \0a";
_utf8_on($val);

my $folder = Lucy::Store::RAMFolder->new;

for my $try ( ( 1 .. 3 ) ) {
    my $schema = new_schema();

    ok( my $indexer = Lucy::Index::Indexer->new(
            index  => $folder,
            schema => $schema,
            create => 1,
        ),
        "create indexer $try"
    );
    $indexer->add_doc(
        {   text     => $val,
            unstored => $val,
            empty    => '',
        }
    );
    $indexer->commit;
}
