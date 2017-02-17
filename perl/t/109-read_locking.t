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

use Test::More tests => 14;

package NonMergingIndexManager;
use base qw( Lucy::Index::IndexManager );
sub recycle { [] }

package main;
use Scalar::Util qw( blessed );

use Lucy::Test::TestUtils qw( create_index );
use Lucy::Util::IndexFileNames qw( latest_snapshot );

my $folder  = create_index(qw( a b c ));
my $schema  = Lucy::Test::TestSchema->new;
my $indexer = Lucy::Index::Indexer->new(
    index   => $folder,
    schema  => $schema,
    manager => Lucy::Index::IndexManager->new,
    create  => 1,
);
$indexer->delete_by_term( field => 'content', term => $_ ) for qw( a b c );
$indexer->add_doc( { content => 'x' } );

# Artificially create snapshot lock.
my $outstream = $folder->open_out('locks/snapshot_1.lock')
    or die Clownfish->error;
$outstream->print("{}");
$outstream->close;

$indexer->commit;

ok( $folder->exists('locks/snapshot_1.lock'),
    "Indexer doesn't delete snapshot lock when it can't get it" );
my $num_ds_files = grep {m/documents\.dat$/} @{ $folder->list_r };
cmp_ok( $num_ds_files, '>', 1,
    "Indexer doesn't process deletions when it can't get deletion lock" );

my $num_snap_files = grep {m/snapshot.*\.json/} @{ $folder->list_r };
is( $num_snap_files, 2, "didn't zap the old snap file" );

my $reader;
SKIP: {
    skip( "IndexReader opening failure leaks", 1 )
        if $ENV{LUCY_VALGRIND};
    my $snapshot = Lucy::Index::Snapshot->new;
    $snapshot->read_file(
        folder => $folder,
        path   => 'snapshot_1.json',
    );
    eval {
        $reader = Lucy::Index::IndexReader->open(
            index    => $folder,
            snapshot => $snapshot,
            manager  => Lucy::Index::IndexManager->new( host => 'me' ),
        );
    };
    ok( blessed($@) && $@->isa("Lucy::Store::LockErr"),
        "IndexReader dies if it can't get snapshot lock"
    );
}
$folder->delete('locks/snapshot_1.lock')
    or die "Can't delete 'snapshot_1.lock'";

Test_race_condition_1: {
    my $latest_snapshot_file = latest_snapshot($folder);

    # Artificially set up situation where the index was updated and files
    # PolyReader was expecting to see were zapped after a snapshot file was
    # picked.
    $folder->rename( from => $latest_snapshot_file, to => 'temp' );
    $folder->rename(
        from => 'seg_1',
        to   => 'seg_1.hidden',
    );
    Lucy::Index::IndexReader::set_race_condition_debug1(
        Clownfish::String->new($latest_snapshot_file) );

    $reader = Lucy::Index::IndexReader->open(
        index   => $folder,
        manager => Lucy::Index::IndexManager->new( host => 'me' ),
    );
    is( $reader->doc_count, 1,
        "reader overcomes race condition of index update after read lock" );
    is( Lucy::Index::IndexReader::debug1_num_passes(),
        2, "reader retried before succeeding" );

    # Clean up our artificial mess.
    $folder->rename(
        from => 'seg_1.hidden',
        to   => 'seg_1',
    );
    Lucy::Index::IndexReader::set_race_condition_debug1(undef);

    $reader->close;
}

# Start over with one segment.
$folder = create_index(qw( a b c x ));

{
    # Add a second segment and delete one doc from existing segment.
    $indexer = Lucy::Index::Indexer->new(
        schema  => $schema,
        index   => $folder,
        manager => NonMergingIndexManager->new,
    );
    $indexer->add_doc( { content => 'foo' } );
    $indexer->add_doc( { content => 'bar' } );
    $indexer->delete_by_term( field => 'content', term => 'x' );
    $indexer->commit;

    # Delete a doc from the second seg and increase del gen on first seg.
    $indexer = Lucy::Index::Indexer->new(
        schema  => $schema,
        index   => $folder,
        manager => NonMergingIndexManager->new,
    );
    $indexer->delete_by_term( field => 'content', term => 'a' );
    $indexer->delete_by_term( field => 'content', term => 'foo' );
    $indexer->commit;
}

# Establish read lock.
$reader = Lucy::Index::IndexReader->open(
    index   => $folder,
    manager => Lucy::Index::IndexManager->new( host => 'me' ),
);

$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->delete_by_term( field => 'content', term => 'a' );
$indexer->optimize;
$indexer->commit;

my $files = $folder->list_r;
$num_snap_files = scalar grep {m/snapshot_\w+\.json$/} @$files;
is( $num_snap_files, 2, "lock preserved last snapshot file" );
my $num_del_files = scalar grep {m/deletions-seg_1\.bv$/} @$files;
is( $num_del_files, 2, "outdated but locked del files survive" );
ok( $folder->exists('seg_3/deletions-seg_1.bv'),
    "first correct old del file" );
ok( $folder->exists('seg_3/deletions-seg_2.bv'),
    "second correct old del file" );
$num_ds_files = scalar grep {m/documents\.dat$/} @$files;
cmp_ok( $num_ds_files, '>', 1, "segment data files preserved" );

undef $reader;
$indexer = Lucy::Index::Indexer->new(
    index  => $folder,
    schema => $schema,
);
$indexer->optimize;
$indexer->commit;

$files = $folder->list_r;
$num_del_files = scalar grep {m/deletions/} @$files;
is( $num_del_files, 0, "lock freed, del files optimized away" );
$num_snap_files = scalar grep {m/snapshot_\w+\.json$/} @$files;
is( $num_snap_files, 1, "lock freed, now only one snapshot file" );
$num_ds_files = scalar grep {m/documents\.dat$/} @$files;
is( $num_ds_files, 1, "lock freed, now only one ds file" );
