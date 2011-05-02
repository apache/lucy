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

use Test::More tests => 5;

package TestAnalyzer;
use base qw( Lucy::Analysis::Analyzer );
sub transform { $_[1] }

package main;
use Encode qw( _utf8_on );
use Lucy::Test;

sub new_schema {
    my $schema     = Lucy::Plan::Schema->new;
    my $analyzer   = TestAnalyzer->new;
    my $fulltext   = Lucy::Plan::FullTextType->new( analyzer => $analyzer );
    my $bin        = Lucy::Plan::BlobType->new( stored => 1 );
    my $not_stored = Lucy::Plan::FullTextType->new(
        analyzer => $analyzer,
        stored   => 0,
    );
    my $float64 = Lucy::Plan::Float64Type->new( indexed => 0 );
    $schema->spec_field( name => 'text',     type => $fulltext );
    $schema->spec_field( name => 'bin',      type => $bin );
    $schema->spec_field( name => 'unstored', type => $not_stored );
    $schema->spec_field( name => 'float64',  type => $float64 );
    $schema->spec_field( name => 'empty',    type => $fulltext );
    return $schema;
}

# This valid UTF-8 string includes skull and crossbones, null byte -- however,
# the binary value is not flagged as UTF-8.
my $bin_val = my $val = "a b c \xe2\x98\xA0 \0a";
_utf8_on($val);

my $folder = Lucy::Store::RAMFolder->new;
my $schema = new_schema();

my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
    create => 1,
);
$indexer->add_doc(
    {   text     => $val,
        bin      => $bin_val,
        unstored => $val,
        empty    => '',
        float64  => 2.0,
    }
);
$indexer->commit;

my $snapshot = Lucy::Index::Snapshot->new->read_file( folder => $folder );
my $segment = Lucy::Index::Segment->new( number => 1 );
$segment->read_file($folder);
my $doc_reader = Lucy::Index::DefaultDocReader->new(
    schema   => $schema,
    folder   => $folder,
    snapshot => $snapshot,
    segments => [$segment],
    seg_tick => 0,
);

my $doc = $doc_reader->fetch_doc(0);

is( $doc->{text},     $val,     "text" );
is( $doc->{bin},      $bin_val, "bin" );
is( $doc->{unstored}, undef,    "unstored" );
is( $doc->{empty},    '',       "empty" );
is( $doc->{float64},  2.0,      "float64" );
