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

package NonMergingIndexManager;
use base qw( Lucy::Index::IndexManager );

sub recycle {
    return Lucy::Object::VArray->new( capacity => 0 );
}

package SortSchema;
use base qw( Lucy::Plan::Schema );

sub new {
    my $self          = shift->SUPER::new(@_);
    my $fulltext_type = Lucy::Plan::FullTextType->new(
        analyzer => Lucy::Analysis::RegexTokenizer->new,
        sortable => 1,
    );
    my $string_type = Lucy::Plan::StringType->new( sortable => 1 );
    my $unsortable = Lucy::Plan::StringType->new;
    $self->spec_field( name => 'name',   type => $fulltext_type );
    $self->spec_field( name => 'speed',  type => $string_type );
    $self->spec_field( name => 'weight', type => $string_type );
    $self->spec_field( name => 'home',   type => $string_type );
    $self->spec_field( name => 'cat',    type => $string_type );
    $self->spec_field( name => 'wheels', type => $string_type );
    $self->spec_field( name => 'unused', type => $string_type );
    $self->spec_field( name => 'nope',   type => $unsortable );
    return $self;
}

package main;
use Lucy::Test;
use Test::More tests => 57;

# Force frequent flushes.
Lucy::Index::SortWriter::set_default_mem_thresh(100);

my $airplane = {
    name   => 'airplane',
    speed  => '0200',
    weight => '8000',
    home   => 'air',
    cat    => 'vehicle',
    wheels => 3,
    nope   => 'nyet',
};
my $bike = {
    name   => 'bike',
    speed  => '0015',
    weight => '0025',
    home   => 'land',
    cat    => 'vehicle',
    wheels => 2,
};
my $car = {
    name   => 'car',
    speed  => '0070',
    weight => '3000',
    home   => 'land',
    cat    => 'vehicle',
    wheels => 4,
};
my $dirigible = {
    name   => 'dirigible',
    speed  => '0040',
    weight => '0000',
    home   => 'air',
    cat    => 'vehicle',
    # no "wheels" field -- test NULL/undef
};
my $elephant = {
    name   => 'elephant',
    speed  => '0020',
    weight => '6000',
    home   => 'land',
    cat    => 'vehicle',
    # no "wheels" field -- test NULL/undef
};

my $folder  = Lucy::Store::RAMFolder->new;
my $schema  = SortSchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);

# Add vehicles.
$indexer->add_doc($_) for ( $airplane, $bike, $car );

$indexer->commit;

my $polyreader  = Lucy::Index::IndexReader->open( index => $folder );
my $seg_reader  = $polyreader->get_seg_readers->[0];
my $sort_reader = $seg_reader->obtain("Lucy::Index::SortReader");
my $doc_reader  = $seg_reader->obtain("Lucy::Index::DocReader");
my $segment     = $seg_reader->get_segment;

for my $field (qw( name speed weight home cat wheels )) {
    my $field_num = $segment->field_num($field);
    ok( $folder->exists("seg_1/sort-$field_num.ord"),
        "sort files written for $field" );
    my $sort_cache = $sort_reader->fetch_sort_cache($field);
    for ( 1 .. $seg_reader->doc_max ) {
        is( $sort_cache->value( ord => $sort_cache->ordinal($_) ),
            $doc_reader->fetch_doc($_)->{$field},
            "correct cached value doc $_ "
        );
    }
}

for my $field (qw( unused nope )) {
    my $field_num = $segment->field_num($field);
    ok( !$folder->exists("seg_1/sort-$field_num.ord"),
        "no sort files written for $field" );
}

# Add a second segment.
$indexer = Lucy::Index::Indexer->new(
    index   => $folder,
    schema  => $schema,
    manager => NonMergingIndexManager->new,
);
$indexer->add_doc($dirigible);
$indexer->commit;

# Consolidate everything, to test merging.
$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->delete_by_term( field => 'name', term => 'bike' );
$indexer->add_doc($elephant);
$indexer->optimize;
$indexer->commit;

my $num_old_seg_files = scalar grep {m/seg_[12]/} @{ $folder->list_r };
is( $num_old_seg_files, 0, "all files from earlier segments zapped" );

$polyreader  = Lucy::Index::IndexReader->open( index => $folder );
$seg_reader  = $polyreader->get_seg_readers->[0];
$sort_reader = $seg_reader->obtain("Lucy::Index::SortReader");
$doc_reader  = $seg_reader->obtain("Lucy::Index::DocReader");
$segment     = $seg_reader->get_segment;

for my $field (qw( name speed weight home cat wheels )) {
    my $field_num = $segment->field_num($field);
    ok( $folder->exists("seg_3/sort-$field_num.ord"),
        "sort files written for $field" );
    my $sort_cache = $sort_reader->fetch_sort_cache($field);
    for ( 1 .. $seg_reader->doc_max ) {
        is( $sort_cache->value( ord => $sort_cache->ordinal($_) ),
            $doc_reader->fetch_doc($_)->{$field},
            "correct cached value field $field doc $_ "
        );
    }
}
